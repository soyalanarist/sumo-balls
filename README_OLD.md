# Sumo Balls - Multiplayer Arena Game

A fast-paced multiplayer arena game where players try to knock each other out of a shrinking arena.

## Features

- **Online Multiplayer**: Authoritative server with client-side prediction
- **Smooth 60 FPS Gameplay**: Optimized networking with interpolation
- **Physics-Based Combat**: Momentum-based collisions
- **Shrinking Arena**: Arena shrinks over time, increasing intensity
- **Particle Effects**: Explosions and visual feedback

## Building the Game

### Dependencies

- **C++17** compiler (g++ or clang++)
- **CMake** 3.10 or higher
- **SFML** 2.5 or higher (Graphics, Window, System)
- **ENet** 1.3 (automatically fetched by CMake)

### Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install build-essential cmake libsfml-dev
```

### Arch Linux

```bash
sudo pacman -S base-devel cmake sfml
```

### Build Instructions

```bash
# Clone the repository
cd sumo-balls

# Create build directory and compile
mkdir -p build
cd build
cmake ..
make -j$(nproc)
```

This creates two executables:
- `sumo_balls` - The game client
- `sumo_balls_server` - The dedicated server

## Running the Game

### Starting a Server

```bash
cd build
./sumo_balls_server <port>
```

Example:
```bash
./sumo_balls_server 7777
```

The server will:
- Listen on the specified port (default: 7777)
- Accept player connections
- Run authoritative game simulation
- Broadcast game state at 33 Hz (every 30ms)
- Handle player collisions and arena boundaries

**Firewall Setup**: Ensure the port is open:
```bash
# Ubuntu/Debian with UFW
sudo ufw allow 7777/udp

# Or using iptables
sudo iptables -A INPUT -p udp --dport 7777 -j ACCEPT
```

### Running the Client

#### Local/Offline Mode
```bash
cd build
./sumo_balls
```
- Navigate menu with mouse
- Click "Play" for local game
- Controls: WASD to move

#### Online Mode

Edit `config.json` in the game directory:
```json
{
  "leftyMode": false,
  "fullscreen": false,
  "playerColorIndex": 5,
  "onlineEnabled": true,
  "onlineHost": "127.0.0.1",    # Server IP address
  "onlinePort": 7777            # Server port
}
```

Then launch:
```bash
cd build
SUMO_ONLINE=1 ./sumo_balls
```

Or click "Play Online" from the main menu.

### Controls

- **WASD**: Movement (default)
- **Arrow Keys**: Alternative movement controls
- **IJKL**: Left-handed mode (enable in settings)
- **ESC**: Pause/Menu

## Network Configuration

### Server Requirements

- **Bandwidth**: ~10 KB/s per player (upstream)
- **Tick Rate**: 500 Hz (2ms simulation step)
- **Snapshot Rate**: 33 Hz (30ms between broadcasts)
- **Protocol**: UDP via ENet

### Client Requirements

- **Bandwidth**: ~5 KB/s (downstream)
- **Latency**: Best with <100ms RTT
- **Frame Rate**: Capped at 60 FPS

### Port Forwarding

If hosting from home network, forward UDP port on your router:
1. Access router admin panel (usually 192.168.1.1)
2. Find "Port Forwarding" or "Virtual Servers"
3. Add rule: External Port 7777 UDP → Internal IP:7777

### Finding Your Server IP

**Local Network**:
```bash
ip addr show | grep "inet "
```

**Public IP** (for internet play):
```bash
curl ifconfig.me
```

## Configuration Files

### config.json

Located in the game root directory:
- `leftyMode`: Use IJKL instead of WASD
- `fullscreen`: Fullscreen mode
- `playerColorIndex`: Your player color (0-7)
- `onlineEnabled`: Enable online features
- `onlineHost`: Server IP address
- `onlinePort`: Server port

### Server Configuration

Edit `src/server_main.cpp` before building to adjust:
- `snapshotTimer`: Snapshot broadcast interval (default: 0.03f = 30ms)
- `sleep`: Server tick sleep time (default: 2ms)

## Deployment Options

### Local Network (LAN Party)

1. One player runs server:
   ```bash
   ./sumo_balls_server 7777
   ```

2. Get server's local IP:
   ```bash
   hostname -I
   ```

3. Other players edit their `config.json`:
   ```json
   "onlineHost": "192.168.x.x"  # Server's IP
   ```

4. All players launch:
   ```bash
   SUMO_ONLINE=1 ./sumo_balls
   ```

### Internet Play

1. **Server Host**:
   - Run server on machine with public IP or port forwarding
   - Share public IP with players
   
2. **Players**:
   - Update `config.json` with server's public IP
   - Launch with `SUMO_ONLINE=1`

### Cloud Deployment (VPS/Dedicated Server)

```bash
# On server (e.g., AWS, DigitalOcean, Linode)
apt-get update
apt-get install build-essential cmake libsfml-dev

# Build
git clone <repo>
cd sumo-balls
mkdir build && cd build
cmake ..
make sumo_balls_server

# Run with systemd (optional)
# Create /etc/systemd/system/sumo-balls.service
```

Example systemd service:
```ini
[Unit]
Description=Sumo Balls Game Server
After=network.target

[Service]
Type=simple
User=gameserver
WorkingDirectory=/opt/sumo-balls/build
ExecStart=/opt/sumo-balls/build/sumo_balls_server 7777
Restart=always

[Install]
WantedBy=multi-user.target
```

## Troubleshooting

### Players Not Visible
- Ensure server is running and accessible
- Check firewall rules allow UDP traffic
- Verify `config.json` has correct server IP/port

### Lag/High Latency
- Check RTT display in-game (top-left corner)
- Reduce network distance to server
- Ensure server has adequate CPU/bandwidth

### Build Errors
```bash
# Missing SFML
sudo apt-get install libsfml-dev

# CMake too old
# Download newer CMake from cmake.org
```

### Connection Refused
- Verify server is running: `netstat -ulnp | grep 7777`
- Check firewall: `sudo ufw status`
- Ensure port forwarding is configured

## Architecture

- **Client-Server Model**: Authoritative server, client-side prediction
- **Networking**: ENet reliable UDP
- **Physics**: Custom collision detection and momentum system
- **Rendering**: SFML with 60 FPS frame limiting
- **Interpolation**: Smoothstep interpolation with 0ms delay for responsive gameplay

## Performance

- **Server**: ~1-2% CPU per player on modern hardware
- **Client**: ~5-10% CPU, integrated graphics sufficient
- **Memory**: <50 MB per client/server instance

## License

[Add your license here]

## Credits

[Add credits here]
