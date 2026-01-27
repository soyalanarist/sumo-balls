#include "../TestFramework.h"
#include "core/Screen.h"
#include "core/ScreenStack.h"
#include "core/ScreenTransition.h"
#include <SDL2/SDL.h>
#include <memory>
#include <iostream>
#include <vector>

// ============================================================================
// Minimal ScreenStack implementation for testing (no menu dependencies)
// ============================================================================

ScreenStack::ScreenStack(void*, void*) {}

void ScreenStack::push(std::unique_ptr<Screen> screen) {
    screens.push_back(std::move(screen));
}

void ScreenStack::pop() {
    if (!screens.empty()) {
        screens.pop_back();
    }
}

void ScreenStack::clear() {
    while (!screens.empty()) {
        screens.pop_back();
    }
}

bool ScreenStack::handleInput(const SDL_Event& event) {
    if (screens.empty()) return false;
    
    // Event bubbling: Process input top-to-bottom through overlays
    for (int i = static_cast<int>(screens.size()) - 1; i >= 0; --i) {
        if (!screens[i]->wantsInput()) {
            continue;
        }
        
        if (screens[i]->handleInput(event)) {
            return true;
        }
        
        if (!screens[i]->isOverlay()) {
            break;
        }
    }
    
    return false;
}

void ScreenStack::update() {
    if (screens.empty()) return;

    for (int i = static_cast<int>(screens.size()) - 1; i >= 0; --i) {
        screens[i]->update();
        if (!screens[i]->isOverlay()) break;
    }
}

void ScreenStack::render() {
    for (int i = 0; i < static_cast<int>(screens.size()); ++i) {
        bool hasNonOverlayAbove = false;
        for (int j = i + 1; j < static_cast<int>(screens.size()); ++j) {
            if (!screens[j]->isOverlay()) {
                hasNonOverlayAbove = true;
                break;
            }
        }
        if (!hasNonOverlayAbove) {
            screens[i]->render();
        }
    }
}

// ============================================================================
// Mock Screen for Testing
// ============================================================================

class MockScreen : public Screen {
public:
    MockScreen(const std::string& name = "Mock", bool isOverlayScreen = false)
        : screenName(name), overlayScreen(isOverlayScreen), updateCount(0), 
          inputCount(0), inputConsumed(false) {}

    void update() override { updateCount++; }
    void render() override {}
    bool isOverlay() const override { return overlayScreen; }
    bool handleInput(const SDL_Event& event) override {
        (void)event;
        inputCount++;
        return inputConsumed;
    }
    ScreenTransition getTransition() const override { return action; }
    void resetTransition() override { action = ScreenTransition::NONE; }
    
    int getUpdateCount() const { return updateCount; }
    int getInputCount() const { return inputCount; }
    void setInputConsumed(bool consumed) { inputConsumed = consumed; }
    void setAction(ScreenTransition a) { action = a; }

private:
    std::string screenName;
    bool overlayScreen;
    int updateCount;
    int inputCount;
    bool inputConsumed;
    ScreenTransition action = ScreenTransition::NONE;
};

class NonInputScreen : public MockScreen {
public:
    NonInputScreen() : MockScreen("NonInput") {}
    bool wantsInput() const override { return false; }
};

// ============================================================================
// Tests
// ============================================================================

bool testScreenStackBasicOps(std::string& errorMsg) {
    ScreenStack stack(nullptr, nullptr);
    TEST_ASSERT(stack.isEmpty(), "Stack should start empty");
    stack.push(std::make_unique<MockScreen>("S1"));
    TEST_ASSERT(!stack.isEmpty(), "Stack should not be empty");
    stack.clear();
    TEST_ASSERT(stack.isEmpty(), "Stack should be empty after clear");
    return true;
}

bool testUpdateOnlyTopScreenWhenNonOverlay(std::string& errorMsg) {
    ScreenStack stack(nullptr, nullptr);
    auto s1 = std::make_unique<MockScreen>("S1", false);
    auto s2 = std::make_unique<MockScreen>("S2", false);
    auto* p1 = s1.get();
    auto* p2 = s2.get();
    stack.push(std::move(s1));
    stack.push(std::move(s2));
    stack.update();
    TEST_EQUAL(0, p1->getUpdateCount(), "Bottom non-overlay should not update");
    TEST_EQUAL(1, p2->getUpdateCount(), "Top screen should update");
    return true;
}

bool testUpdateAllWhenOverlays(std::string& errorMsg) {
    ScreenStack stack(nullptr, nullptr);
    auto base = std::make_unique<MockScreen>("Base", false);
    auto over1 = std::make_unique<MockScreen>("Over1", true);
    auto over2 = std::make_unique<MockScreen>("Over2", true);
    auto* pb = base.get();
    auto* p1 = over1.get();
    auto* p2 = over2.get();
    stack.push(std::move(base));
    stack.push(std::move(over1));
    stack.push(std::move(over2));
    stack.update();
    TEST_EQUAL(1, pb->getUpdateCount(), "Base should update");
    TEST_EQUAL(1, p1->getUpdateCount(), "Overlay 1 should update");
    TEST_EQUAL(1, p2->getUpdateCount(), "Overlay 2 should update");
    return true;
}

bool testInputTopScreenFirst(std::string& errorMsg) {
    ScreenStack stack(nullptr, nullptr);
    auto s1 = std::make_unique<MockScreen>("S1", false);
    auto s2 = std::make_unique<MockScreen>("S2", false);
    auto* p1 = s1.get();
    auto* p2 = s2.get();
    p1->setInputConsumed(false);
    p2->setInputConsumed(false);
    stack.push(std::move(s1));
    stack.push(std::move(s2));
    SDL_Event evt{};
    bool handled = stack.handleInput(evt);
    TEST_ASSERT(!handled, "Not consumed");
    TEST_EQUAL(1, p2->getInputCount(), "Top gets input");
    TEST_EQUAL(0, p1->getInputCount(), "Bottom doesn't");
    return true;
}

bool testInputBubblesWhenNotConsumed(std::string& errorMsg) {
    ScreenStack stack(nullptr, nullptr);
    auto over1 = std::make_unique<MockScreen>("Over1", true);
    auto over2 = std::make_unique<MockScreen>("Over2", true);
    auto* p1 = over1.get();
    auto* p2 = over2.get();
    p1->setInputConsumed(true);
    p2->setInputConsumed(false);
    stack.push(std::move(over1));
    stack.push(std::move(over2));
    SDL_Event evt{};
    bool handled = stack.handleInput(evt);
    TEST_ASSERT(handled, "Should be consumed");
    TEST_EQUAL(1, p2->getInputCount(), "Top gets input");
    TEST_EQUAL(1, p1->getInputCount(), "Bottom gets input when top not consumed");
    return true;
}

bool testInputStopsAtNonOverlay(std::string& errorMsg) {
    ScreenStack stack(nullptr, nullptr);
    auto base = std::make_unique<MockScreen>("Base", false);
    auto over = std::make_unique<MockScreen>("Over", true);
    auto* pb = base.get();
    auto* po = over.get();
    pb->setInputConsumed(false);
    po->setInputConsumed(false);
    stack.push(std::move(base));
    stack.push(std::move(over));
    SDL_Event evt{};
    stack.handleInput(evt);
    TEST_EQUAL(1, po->getInputCount(), "Overlay gets input");
    // When overlay is above base and doesn't consume, base gets input too
    TEST_EQUAL(1, pb->getInputCount(), "Base gets input when overlay doesn't consume");
    return true;
}

bool testInputSkipsNoWantInputScreens(std::string& errorMsg) {
    ScreenStack stack(nullptr, nullptr);
    auto norm = std::make_unique<MockScreen>("Norm", true);
    auto noinp = std::make_unique<NonInputScreen>();
    auto* pn = norm.get();
    auto* pni = noinp.get();
    pn->setInputConsumed(false);
    stack.push(std::move(norm));
    stack.push(std::move(noinp));
    SDL_Event evt{};
    stack.handleInput(evt);
    TEST_EQUAL(0, pni->getInputCount(), "No-input screen skipped");
    TEST_EQUAL(1, pn->getInputCount(), "Next screen gets input");
    return true;
}

bool testESCKeyEvent(std::string& errorMsg) {
    ScreenStack stack(nullptr, nullptr);
    auto scr = std::make_unique<MockScreen>("Game", false);
    auto* ps = scr.get();
    stack.push(std::move(scr));
    SDL_Event evt{};
    evt.type = SDL_KEYDOWN;
    evt.key.keysym.sym = SDLK_ESCAPE;
    ps->setInputConsumed(false);
    bool h1 = stack.handleInput(evt);
    TEST_ASSERT(!h1, "Not consumed");
    TEST_EQUAL(1, ps->getInputCount(), "Received");
    ps->setInputConsumed(true);
    bool h2 = stack.handleInput(evt);
    TEST_ASSERT(h2, "Consumed when screen handles it");
    return true;
}

bool testScreenTransitionWorks(std::string& errorMsg) {
    ScreenStack stack(nullptr, nullptr);
    auto scr = std::make_unique<MockScreen>("Menu");
    auto* ps = scr.get();
    ps->setAction(ScreenTransition::TO_MAIN_MENU);
    stack.push(std::move(scr));
    ScreenTransition a1 = ps->getTransition();
    TEST_EQUAL(static_cast<int>(ScreenTransition::TO_MAIN_MENU), static_cast<int>(a1), "Action set");
    ps->resetTransition();
    ScreenTransition a2 = ps->getTransition();
    TEST_EQUAL(static_cast<int>(ScreenTransition::NONE), static_cast<int>(a2), "Action reset");
    return true;
}

bool testPauseMenuScenario(std::string& errorMsg) {
    ScreenStack stack(nullptr, nullptr);
    auto game = std::make_unique<MockScreen>("Game", false);
    auto pause = std::make_unique<MockScreen>("Pause", true);
    auto* pg = game.get();
    auto* pp = pause.get();
    pg->setInputConsumed(false);
    pp->setInputConsumed(true);
    stack.push(std::move(game));
    stack.push(std::move(pause));
    
    stack.update();
    TEST_EQUAL(1, pg->getUpdateCount(), "Game updates");
    TEST_EQUAL(1, pp->getUpdateCount(), "Pause updates");
    
    SDL_Event evt{};
    bool handled = stack.handleInput(evt);
    TEST_ASSERT(handled, "Pause handles input");
    TEST_EQUAL(1, pp->getInputCount(), "Pause gets input");
    TEST_EQUAL(0, pg->getInputCount(), "Game doesn't");
    return true;
}

bool testMultipleOverlayBubble(std::string& errorMsg) {
    ScreenStack stack(nullptr, nullptr);
    auto game = std::make_unique<MockScreen>("Game", false);
    auto dlg1 = std::make_unique<MockScreen>("Dlg1", true);
    auto dlg2 = std::make_unique<MockScreen>("Dlg2", true);
    auto* pg = game.get();
    auto* pd1 = dlg1.get();
    auto* pd2 = dlg2.get();
    
    pg->setInputConsumed(false);
    pd1->setInputConsumed(true);
    pd2->setInputConsumed(false);
    
    stack.push(std::move(game));
    stack.push(std::move(dlg1));
    stack.push(std::move(dlg2));
    
    SDL_Event evt{};
    bool handled = stack.handleInput(evt);
    TEST_ASSERT(handled, "Consumed");
    TEST_EQUAL(1, pd2->getInputCount(), "Top gets input");
    TEST_EQUAL(1, pd1->getInputCount(), "Middle gets input");
    TEST_EQUAL(0, pg->getInputCount(), "Game doesn't");
    return true;
}

bool testEmptyStack(std::string& errorMsg) {
    ScreenStack stack(nullptr, nullptr);
    SDL_Event evt{};
    bool handled = stack.handleInput(evt);
    TEST_ASSERT(!handled, "Empty doesn't handle");
    return true;
}

bool testSingleScreen(std::string& errorMsg) {
    ScreenStack stack(nullptr, nullptr);
    auto scr = std::make_unique<MockScreen>("Only");
    auto* ps = scr.get();
    ps->setInputConsumed(true);
    stack.push(std::move(scr));
    SDL_Event evt{};
    bool handled = stack.handleInput(evt);
    TEST_ASSERT(handled, "Single handles");
    TEST_EQUAL(1, ps->getInputCount(), "Gets input");
    return true;
}

// ============================================================================
// Auto-registration
// ============================================================================

namespace {
    struct ScreenTransitionTests {
        ScreenTransitionTests() {
            test::TestSuite& s = test::TestSuite::instance();
            s.registerTest("ScreenStack::BasicOps", testScreenStackBasicOps);
            s.registerTest("ScreenStack::UpdateOnlyTopNonOverlay", testUpdateOnlyTopScreenWhenNonOverlay);
            s.registerTest("ScreenStack::UpdateAllWithOverlays", testUpdateAllWhenOverlays);
            s.registerTest("ScreenStack::InputTopFirst", testInputTopScreenFirst);
            s.registerTest("ScreenStack::InputBubblesNotConsumed", testInputBubblesWhenNotConsumed);
            s.registerTest("ScreenStack::InputStopsNonOverlay", testInputStopsAtNonOverlay);
            s.registerTest("ScreenStack::InputSkipsNoWantInput", testInputSkipsNoWantInputScreens);
            s.registerTest("ScreenStack::ESCKeyEvent", testESCKeyEvent);
            s.registerTest("ScreenStack::ScreenTransition", testScreenTransitionWorks);
            s.registerTest("ScreenStack::PauseMenuScenario", testPauseMenuScenario);
            s.registerTest("ScreenStack::MultipleOverlayBubble", testMultipleOverlayBubble);
            s.registerTest("ScreenStack::EmptyStack", testEmptyStack);
            s.registerTest("ScreenStack::SingleScreen", testSingleScreen);
        }
    } tests;
}
