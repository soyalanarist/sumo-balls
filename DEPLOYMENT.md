# Deployment Guide

## Quick Start for Players

### Option 1: Play on LAN (Same Network)

**Server Host:**
```bash
./run-server.sh 7777
```

Get your local IP:
```bash
hostname -I
# Example output: 192.168.1.100
```

**Other Players:**

Edit `config.json`:
```json
{
  "onlineHost": "192.168.1.100",
  "onlinePort": 7777
}
```

Run:
```bash
./run-client.sh
```

### Option 2: Play Over Internet

**Requirements:**
- Server needs public IP or port forwarding
- UDP port 7777 open on firewall

**Server Setup:**

1. Find your public IP:
   ```bash
   curl ifconfig.me
   ```

2. Forward port on router:
   - Log into router (usually 192.168.1.1 or 192.168.0.1)
   - Find "Port Forwarding" or "NAT"
   - Add: External 7777/UDP → Internal <your-local-ip>:7777

3. Allow through firewall:
   ```bash
   sudo ufw allow 7777/udp
   ```

4. Start server:
   ```bash
   ./run-server.sh 7777
   ```

**Players:**

Edit `config.json` with server's public IP:
```json
{
  "onlineHost": "203.0.113.42",
  "onlinePort": 7777
}
```

Run:
```bash
./run-client.sh
```

## Distribution

### Packaging for Distribution

Create a release package:

```bash
# Build release version
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
cd ..

# Create package
mkdir -p sumo-balls-release
cp build/sumo_balls sumo-balls-release/
cp build/sumo_balls_server sumo-balls-release/
cp -r assets sumo-balls-release/
cp config.json sumo-balls-release/
cp run-server.sh sumo-balls-release/
cp run-client.sh sumo-balls-release/
cp README.md sumo-balls-release/

# Create archive
tar -czf sumo-balls-linux-x64.tar.gz sumo-balls-release/
```

Players can then:
```bash
tar -xzf sumo-balls-linux-x64.tar.gz
cd sumo-balls-release
./run-client.sh
```

### Cloud/VPS Hosting

For dedicated server hosting (AWS, DigitalOcean, etc.):

```bash
# On server
git clone <your-repo>
cd sumo-balls

# Install dependencies
sudo apt-get update
sudo apt-get install -y build-essential cmake libsfml-dev

# Build server only
mkdir -p build && cd build
cmake ..
make sumo_balls_server

# Run
./sumo_balls_server 7777
```

#### Systemd Service (keep server running)

Create `/etc/systemd/system/sumo-balls.service`:

```ini
[Unit]
Description=Sumo Balls Game Server
After=network.target

[Service]
Type=simple
User=ubuntu
WorkingDirectory=/home/ubuntu/sumo-balls/build
ExecStart=/home/ubuntu/sumo-balls/build/sumo_balls_server 7777
Restart=always
RestartSec=3

[Install]
WantedBy=multi-user.target
```

Enable and start:
```bash
sudo systemctl enable sumo-balls
sudo systemctl start sumo-balls
sudo systemctl status sumo-balls
```

#### Docker (Optional)

Create `Dockerfile`:
```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libsfml-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN mkdir build && cd build && \
    cmake .. && \
    make sumo_balls_server

EXPOSE 7777/udp

CMD ["./build/sumo_balls_server", "7777"]
```

Build and run:
```bash
docker build -t sumo-balls-server .
docker run -d -p 7777:7777/udp --name sumo-server sumo-balls-server
```

## Testing Multiplayer

### Same Machine (Development)

Not ideal since both windows read same keyboard, but for testing:

Terminal 1:
```bash
./run-server.sh
```

Terminal 2:
```bash
./run-client.sh
```

Terminal 3:
```bash
./run-client.sh
```

### Different Machines (Recommended)

**Machine 1** (Server + Player 1):
```bash
./run-server.sh &
sleep 2
./run-client.sh
```

**Machine 2** (Player 2):
Edit `config.json` to point to Machine 1's IP, then:
```bash
./run-client.sh
```

### With Friends

1. Build the game: `./build.sh`
2. Start server: `./run-server.sh`
3. Share your IP with friends
4. Friends edit their `config.json` and run `./run-client.sh`

## Monitoring

### Server Logs

Server outputs to stdout:
```
Server: Broadcasting snapshot with 2 players, serverTime=12345
```

Redirect to file:
```bash
./run-server.sh 7777 > server.log 2>&1
```

### Client Connection

Client shows RTT in top-left corner when connected.

### Network Statistics

Monitor bandwidth:
```bash
sudo iftop -i eth0 -f "udp port 7777"
```

## Troubleshooting

### Can't Connect

1. Verify server is running:
   ```bash
   netstat -ulnp | grep 7777
   ```

2. Test connectivity:
   ```bash
   nc -u -v <server-ip> 7777
   ```

3. Check firewall:
   ```bash
   sudo ufw status
   ```

### High Lag

- Check RTT display in-game
- Use server closer to players
- Reduce network hops
- Check bandwidth usage

### Server Crashes

- Check logs for errors
- Ensure adequate CPU/memory
- Monitor with `htop`

## Performance Tuning

### Server

Edit [server_main.cpp](src/server_main.cpp):
- `snapshotTimer`: Lower = more updates (higher bandwidth)
- Sleep duration: Lower = more responsive (higher CPU)

### Client

- Lower snapshot interpolation delay for more responsive controls
- Increase for smoother but slightly delayed movement

Current settings (optimized):
- Snapshot rate: 33 Hz
- Interpolation delay: 0ms
- Client prediction: Enabled
