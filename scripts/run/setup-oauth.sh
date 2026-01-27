#!/bin/bash
# Setup Google OAuth for Sumo Balls on WSL
# This script helps configure environment variables for Google OAuth

set -e

echo "=========================================="
echo "Sumo Balls Google OAuth Setup"
echo "=========================================="
echo ""

# Check if credentials are already set
if [ -n "$GOOGLE_CLIENT_ID" ] && [ -n "$GOOGLE_CLIENT_SECRET" ]; then
    echo "✓ Google OAuth credentials already configured"
    echo "  Client ID: ${GOOGLE_CLIENT_ID:0:20}..."
    exit 0
fi

echo "This script will help you set up Google OAuth sign-in."
echo "You will need credentials from Google Cloud Console."
echo ""
echo "Before running this script, please:"
echo "1. Create a Google Cloud project"
echo "2. Create OAuth 2.0 credentials (Desktop application)"
echo "3. Add redirect URI: http://localhost:8888/auth/google/callback"
echo ""
echo "See docs/GOOGLE_OAUTH_SETUP.md for detailed instructions."
echo ""

read -p "Enter your Google Client ID: " CLIENT_ID
if [ -z "$CLIENT_ID" ]; then
    echo "Error: Client ID cannot be empty"
    exit 1
fi

read -sp "Enter your Google Client Secret: " CLIENT_SECRET
echo ""
if [ -z "$CLIENT_SECRET" ]; then
    echo "Error: Client Secret cannot be empty"
    exit 1
fi

# Determine shell profile
SHELL_PROFILE="$HOME/.bashrc"
if [ -f "$HOME/.zshrc" ]; then
    SHELL_PROFILE="$HOME/.zshrc"
fi

# Option 1: Add to shell profile
echo ""
echo "How would you like to save credentials?"
echo "1) Add to $SHELL_PROFILE (permanent, auto-loaded)"
echo "2) Create .env file in coordinator/ (manual source required)"
echo "3) Just print (copy manually to .env)"
read -p "Choose (1-3): " choice

case $choice in
    1)
        # Check if already in file
        if grep -q "GOOGLE_CLIENT_ID" "$SHELL_PROFILE"; then
            echo "Updating existing Google OAuth credentials in $SHELL_PROFILE..."
            # Use sed to update (macOS/Linux compatible)
            sed -i.bak "s|export GOOGLE_CLIENT_ID=.*|export GOOGLE_CLIENT_ID=\"$CLIENT_ID\"|" "$SHELL_PROFILE"
            sed -i.bak "s|export GOOGLE_CLIENT_SECRET=.*|export GOOGLE_CLIENT_SECRET=\"$CLIENT_SECRET\"|" "$SHELL_PROFILE"
            rm -f "$SHELL_PROFILE.bak"
        else
            echo "Adding Google OAuth credentials to $SHELL_PROFILE..."
            cat >> "$SHELL_PROFILE" << EOF

# Google OAuth for Sumo Balls
export GOOGLE_CLIENT_ID="$CLIENT_ID"
export GOOGLE_CLIENT_SECRET="$CLIENT_SECRET"
EOF
        fi
        echo "✓ Credentials saved to $SHELL_PROFILE"
        echo "  Reload with: source $SHELL_PROFILE"
        ;;
    2)
        COORD_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
        ENV_FILE="$COORD_DIR/.env"
        cat > "$ENV_FILE" << EOF
# Google OAuth Credentials
export GOOGLE_CLIENT_ID="$CLIENT_ID"
export GOOGLE_CLIENT_SECRET="$CLIENT_SECRET"
EOF
        chmod 600 "$ENV_FILE"  # Restrict permissions since it has secrets
        echo "✓ Credentials saved to $ENV_FILE"
        echo "  Load with: source $ENV_FILE"
        ;;
    3)
        cat << EOF

Export these commands to set up OAuth:

export GOOGLE_CLIENT_ID="$CLIENT_ID"
export GOOGLE_CLIENT_SECRET="$CLIENT_SECRET"

EOF
        ;;
esac

echo ""
echo "=========================================="
echo "Setup Complete!"
echo "=========================================="
echo ""
echo "Next steps:"
echo "1. Reload your shell: source $SHELL_PROFILE"
echo "2. Start coordinator: cd coordinator && ./run.sh"
echo "3. Build and run client: cd build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make -j4 && ./sumo_balls"
echo "4. Click 'Sign in with Google' button"
echo ""
