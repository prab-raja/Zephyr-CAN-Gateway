# Zephyr Multi-Bus CAN/CAN-FD + UART Diagnostics Gateway

A Zephyr RTOS-based multi-bus gateway with real-time CAN/CAN-FD routing, configurable ID filtering, and UART diagnostic interface.

## Features

- ✅ Compiler flag-based configuration (CAN controller, CAN-FD, filter mode)
- ✅ Runtime configurable whitelist/blacklist filtering
- ✅ UART shell for diagnostics and configuration
- ✅ Real-time CAN message routing
- ✅ Statistics tracking (RX/TX/filtered/errors)
- ✅ Support for STM32F103 (Classic CAN) and STM32H7 (CAN-FD)

## Hardware Support

- **STM32F103**: Dual CAN controllers, Classic CAN only
- **STM32H7**: CAN-FD capable
- **BeagleBone Black**: CAN testing/simulation
- **Raspberry Pi 4**: Additional test node (requires CAN adapter)

## Quick Start with GitHub Codespaces

1. Click "Code" → "Create codespace on main"
2. Wait for environment setup (automatic)
3. Build with predefined configurations:

```bash
# Classic CAN on CAN1 with whitelist
./build_can1.sh

# Classic CAN on CAN2 with whitelist
./build_can2.sh

# CAN-FD on CAN1 with whitelist (STM32H7)
./build_can1_fd.sh

# Classic CAN with blacklist mode
./build_blacklist.sh
```

## Custom Build Configuration

```bash
west build -b <board> app -- \
  -DCONFIG_CAN_CONTROLLER=<1|2> \
  -DCONFIG_USE_CANFD=<y|n> \
  -DCONFIG_FILTER_MODE=<whitelist|blacklist>
```

### Examples

```bash
# STM32F103 with CAN2, blacklist mode
west build -b stm32f103_mini app -- \
  -DCONFIG_CAN_CONTROLLER=2 \
  -DCONFIG_USE_CANFD=n \
  -DCONFIG_FILTER_MODE=blacklist

# STM32H7 with CAN-FD, whitelist mode
west build -b stm32h7 app -- \
  -DCONFIG_CAN_CONTROLLER=1 \
  -DCONFIG_USE_CANFD=y \
  -DCONFIG_FILTER_MODE=whitelist
```

## Flashing

The firmware is built in Codespaces, but flashing must be done locally with connected hardware.

### Using OpenOCD (STM32)

```bash
# Copy binary from Codespaces to local machine
# Then flash:
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
  -c "program build/zephyr/zephyr.elf verify reset exit"
```

### Using west flash (if configured)

```bash
west flash
```

## UART Diagnostics Shell

Connect to the UART console (115200 8N1):

```bash
# Linux
screen /dev/ttyUSB0 115200

# macOS
screen /dev/tty.usbserial-* 115200
```

### Available Commands

#### Statistics
```
stats show          # Display RX/TX/filtered/error counts
stats reset         # Reset all statistics
```

#### Filter Management
```
filter show                    # Show current filter config
filter add 0x123              # Add CAN ID to filter
filter remove 0x123           # Remove CAN ID from filter
filter clear                  # Clear all filters
filter mode whitelist         # Set whitelist mode
filter mode blacklist         # Set blacklist mode
```

## Filter Modes

### Whitelist Mode
- Only CAN IDs in the filter list are routed
- Empty whitelist = reject all messages
- Compile-time default: `-DCONFIG_FILTER_MODE=whitelist`

### Blacklist Mode
- CAN IDs in the filter list are blocked
- Empty blacklist = accept all messages
- Compile-time default: `-DCONFIG_FILTER_MODE=blacklist`

## Project Structure

```
.
├── .devcontainer/          # Codespaces configuration
│   ├── devcontainer.json
│   └── setup.sh
├── app/
│   ├── src/
│   │   ├── main.c                  # Main entry point
│   │   ├── can_router.c            # CAN routing logic
│   │   ├── uart_diagnostics.c      # Shell commands
│   │   └── filter.c                # ID filtering
│   ├── CMakeLists.txt              # Build configuration
│   └── prj.conf                    # Zephyr config
├── build_*.sh              # Quick build scripts
└── README.md
```

## Development Workflow

1. **Write code in Codespaces** - Full Zephyr SDK available
2. **Build firmware** - Use provided scripts or custom commands
3. **Download binary** - Copy from Codespaces to local machine
4. **Flash locally** - Connect hardware and flash
5. **Test/Debug** - Use UART shell for diagnostics

## Compile-Time Configuration Summary

| Flag | Values | Description |
|------|--------|-------------|
| `CONFIG_CAN_CONTROLLER` | `1`, `2` | Select CAN1 or CAN2 |
| `CONFIG_USE_CANFD` | `y`, `n` | Enable CAN-FD support |
| `CONFIG_FILTER_MODE` | `whitelist`, `blacklist` | Default filter mode |

## Runtime Configuration

All filtering can be modified at runtime via UART shell without recompiling:
- Add/remove CAN IDs
- Switch between whitelist/blacklist modes
- View statistics
- Clear filters

## Example Use Cases

### 1. CAN Bus Firewall
- Set blacklist mode
- Add unwanted CAN IDs to filter
- Route only trusted messages

### 2. Selective Message Gateway
- Set whitelist mode
- Add specific CAN IDs for routing
- Block all other traffic

### 3. Multi-Network Bridge
- Configure CAN1 and CAN2 with different filters
- Route messages between networks based on ID

## Troubleshooting

**Build fails**: Check that Codespaces setup completed successfully
**CAN not working**: Verify device tree overlay for your board
**Shell not responding**: Check UART connection and baud rate (115200)
**CAN-FD on STM32F103**: Not supported, use STM32H7 or compatible board

## License

MIT License - See LICENSE file for details

## Contributing

Pull requests welcome! Please ensure:
- Code follows Zephyr coding standards
- Compile-time configurations work correctly
- Shell commands are documented