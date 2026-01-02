#!/bin/bash
set -e

echo "🚀 Setting up Zephyr CAN Gateway development environment..."

# Initialize west workspace
if [ ! -d ".west" ]; then
    echo "📦 Initializing west workspace..."
    west init -l app
    west update
    west zephyr-export
fi

# Install Python dependencies
echo "🐍 Installing Python dependencies..."
pip3 install --user -r ~/zephyrproject/zephyr/scripts/requirements.txt

# Create build configurations directory
mkdir -p build_configs

# Create helper build scripts
cat > build_can1.sh << 'EOF'
#!/bin/bash
west build -b stm32f103_mini app -- \
  -DCONFIG_CAN_CONTROLLER=1 \
  -DCONFIG_USE_CANFD=n \
  -DCONFIG_FILTER_MODE=whitelist
EOF

cat > build_can2.sh << 'EOF'
#!/bin/bash
west build -b stm32f103_mini app -- \
  -DCONFIG_CAN_CONTROLLER=2 \
  -DCONFIG_USE_CANFD=n \
  -DCONFIG_FILTER_MODE=whitelist
EOF

cat > build_can1_fd.sh << 'EOF'
#!/bin/bash
west build -b stm32h7 app -- \
  -DCONFIG_CAN_CONTROLLER=1 \
  -DCONFIG_USE_CANFD=y \
  -DCONFIG_FILTER_MODE=whitelist
EOF

cat > build_blacklist.sh << 'EOF'
#!/bin/bash
west build -b stm32f103_mini app -- \
  -DCONFIG_CAN_CONTROLLER=1 \
  -DCONFIG_USE_CANFD=n \
  -DCONFIG_FILTER_MODE=blacklist
EOF

chmod +x build_*.sh

echo "✅ Development environment ready!"
echo ""
echo "Available build commands:"
echo "  ./build_can1.sh         - Build for CAN1 (Classic CAN, Whitelist)"
echo "  ./build_can2.sh         - Build for CAN2 (Classic CAN, Whitelist)"
echo "  ./build_can1_fd.sh      - Build for CAN1 (CAN-FD, Whitelist) [STM32H7]"
echo "  ./build_blacklist.sh    - Build for CAN1 (Classic CAN, Blacklist)"
echo ""
echo "Custom build:"
echo "  west build -b <board> app -- -DCONFIG_CAN_CONTROLLER=<1|2> -DCONFIG_USE_CANFD=<y|n> -DCONFIG_FILTER_MODE=<whitelist|blacklist>"