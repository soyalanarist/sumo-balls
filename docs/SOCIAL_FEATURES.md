# Social Features Implementation

## Overview
The game now supports a comprehensive social system with user handles/usernames and friend requests. Users can add friends by handle, creating a more social gaming experience.

## Features Implemented

### 1. User Handles System

#### Database Schema
- Added `handle` field: user-chosen display name (nullable, unique when set)
- Added `handle_search` field: normalized lowercase version for case-insensitive lookups
- Added `handle_last_changed` field: timestamp for rate limiting (30-day cooldown between changes)
- Unique index on `handle_search` (excluding NULL values)

#### User Model
```go
type User struct {
    ID                int64
    Username          string      // Internal username (may be auto-generated for OAuth)
    Handle            string      // User-chosen display name
    HandleSearch      string      // Lowercase normalized handle
    PasswordHash      string
    Email             string
    NeedsHandleSetup  bool        // True if OAuth user hasn't chosen handle
    HandleLastChanged *time.Time  // Last handle change (for rate limiting)
    CreatedAt         time.Time
}
```

### 2. Handle Validation

#### Rules
- **Length**: 3-20 characters
- **Characters**: Letters, numbers, underscores only (`^[a-zA-Z0-9_]+$`)
- **Start**: Must start with letter or number (not underscore)
- **Reserved**: Cannot use system-reserved names (admin, moderator, system, etc.)
- **Profanity**: Basic profanity filter (expandable)
- **Uniqueness**: Case-insensitive unique constraint
- **Rate Limit**: Can only change handle once per 30 days

#### Reserved Handles
- admin, moderator, mod, root, system
- official, support, staff, sumoballs
- bot, null, undefined, anonymous, guest

### 3. API Endpoints

#### Check Handle Availability
```
POST /username/check
Content-Type: application/json

{
    "handle": "coolgamer"
}

Response:
{
    "success": true,
    "available": true,
    "message": "Handle is available"
}
```

#### Set/Update Handle
```
POST /username/set
Authorization: Bearer <token>
Content-Type: application/json

{
    "handle": "coolgamer"
}

Response:
{
    "success": true,
    "message": "Handle set successfully",
    "user": {
        "id": 1,
        "username": "player1",
        "handle": "coolgamer",
        "email": "player@example.com",
        "needs_handle_setup": false,
        "created_at": "2026-01-25T14:48:05Z"
    }
}
```

### 4. Friend System Integration

#### Send Friend Request by Handle
Users can now add friends using their handle (case-insensitive):

```
POST /friends/send
Authorization: Bearer <token>
Content-Type: application/json

{
    "friend_username": "BobTheBuilder"  // Can use handle or username
}

Response:
{
    "success": true,
    "message": "Friend request sent"
}
```

The system:
1. First tries to find user by handle (preferred)
2. Falls back to username for backwards compatibility
3. Checks if already friends
4. Prevents self-friending
5. Creates pending friend request

#### Accept Friend Request
```
POST /friends/accept
Authorization: Bearer <token>
Content-Type: application/json

{
    "friend_id": 1
}
```

#### List Friends
```
GET /friends/list
Authorization: Bearer <token>

Response:
{
    "success": true,
    "friends": [
        {
            "id": 1,
            "user_id": 1,
            "friend_id": 2,
            "status": "accepted",
            "friend_username": "bob"
        }
    ]
}
```

#### List Pending Requests
```
GET /friends/pending
Authorization: Bearer <token>

Response:
{
    "success": true,
    "requests": [
        {
            "id": 1,
            "from_user_id": 1,
            "from_username": "alice",
            "created_at": "2026-01-25T14:48:46Z"
        }
    ]
}
```

### 5. Google OAuth Integration

For users logging in with Google:
- If no handle set: `needs_handle_setup` field is `true`
- Auto-generated username based on Google name/email
- User prompted to choose handle on first login
- Suggested handle pre-filled (e.g., "john4827")

### 6. Helper Functions

#### normalizeHandle(handle string)
Converts handle to lowercase and trims whitespace for case-insensitive comparisons.

#### ValidateHandle(handle string)
Comprehensive validation returning `(bool, string)`:
- Checks length requirements
- Validates character set
- Ensures proper start character
- Checks against reserved names
- Applies profanity filter

#### GenerateSuggestedHandle(name, email string)
Creates a suggested handle from user's name or email with random suffix.

## Testing

### Test Scripts
1. **manual_test.sh**: Tests handle availability and setting
2. **friend_test.sh**: Tests complete friend workflow by handle
3. **test_username.sh**: Comprehensive handle system test

### Test Scenarios Covered
✅ User registration with password
✅ Handle availability checking
✅ Handle validation (length, characters, reserved)
✅ Setting handles for users
✅ Duplicate handle prevention (case-insensitive)
✅ Friend request by handle
✅ Friend request acceptance
✅ Friend list retrieval
✅ Backwards compatibility (username fallback)

## Database Migration

The database schema auto-migrates on startup. For existing databases:
- New columns added automatically
- Unique index created with NULL exclusion
- Existing users will have `NULL` handles (can be set later)
- OAuth users flagged with `needs_handle_setup = true`

## Security Considerations

1. **Rate Limiting**: 30-day cooldown on handle changes prevents abuse
2. **Profanity Filter**: Basic filter in place (expand as needed)
3. **Reserved Names**: System names protected
4. **Case-Insensitive**: Prevents similar-looking handles (alice vs ALICE)
5. **Validation**: Strict character set prevents injection/confusion
6. **Authentication**: All handle operations require valid session token

## Future Enhancements

### Suggested Improvements
1. **Discord-style Discriminator**: Add 4-digit suffix (#1234) for non-unique names
2. **Handle History**: Track previous handles for moderation
3. **Custom Profanity Lists**: User-configurable per region/language
4. **Rate Limit Bypass**: Admin override for legitimate changes
5. **Search**: Global handle search with privacy settings
6. **Verification Badges**: Verified/official user indicators
7. **Handle Reservation**: Premium/early-access handle reservation
8. **Unicode Support**: International character support (carefully filtered)

### Client-Side Integration
Next steps for client implementation:
1. Add handle input field to UI
2. Real-time availability checking
3. Handle suggestion dropdown
4. Friend search by handle
5. Display handles in friend lists
6. OAuth flow: prompt for handle on first login

## Files Modified/Added

### New Files
- `coordinator/username.go` - Handle validation and management service

### Modified Files
- `coordinator/types.go` - Added handle fields to User type, new request/response types
- `coordinator/database.go` - Updated schema, queries, and user operations
- `coordinator/friends.go` - Updated friend search to support handles
- `coordinator/main.go` - Wired up username endpoints

### Test Scripts
- `coordinator/manual_test.sh` - Handle testing
- `coordinator/friend_test.sh` - Friend by handle testing
- `coordinator/test_username.sh` - Comprehensive username tests

## Example Usage Flow

### New User Journey
1. User logs in with Google OAuth
2. Backend creates user with auto-generated username
3. `needs_handle_setup` flag set to `true`
4. Client detects flag and shows handle selection screen
5. User checks availability: `POST /username/check {"handle": "coolplayer"}`
6. User sets handle: `POST /username/set {"handle": "coolplayer"}`
7. Profile updated, ready to add friends

### Adding Friends
1. User searches for friend's handle: "awesomefriend"
2. Client checks if valid/exists (optional preview)
3. User sends request: `POST /friends/send {"friend_username": "awesomefriend"}`
4. Friend receives notification via `GET /friends/pending`
5. Friend accepts: `POST /friends/accept {"friend_id": X}`
6. Both users see each other in `GET /friends/list`

## API Summary

| Endpoint | Method | Auth | Description |
|----------|--------|------|-------------|
| `/username/check` | POST | No | Check if handle is available |
| `/username/set` | POST | Yes | Set or update user handle |
| `/friends/send` | POST | Yes | Send friend request by handle |
| `/friends/accept` | POST | Yes | Accept friend request |
| `/friends/list` | GET | Yes | List accepted friends |
| `/friends/pending` | GET | Yes | List pending requests |
| `/friends/remove` | DELETE | Yes | Remove friend |

## Configuration

No additional configuration needed. The system uses:
- Built-in reserved names list
- Built-in profanity filter
- 30-day rate limit (hardcoded, can be made configurable)

## Notes

- Handle system is optional - users can exist without handles
- Internal `username` still used for database lookups and legacy support
- Handle display preferred in UI when available
- OAuth users auto-prompted to set handle
- Password users can set handle optionally
