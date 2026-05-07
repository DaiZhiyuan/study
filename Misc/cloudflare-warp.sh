#!/bin/bash

# WARP One-Click Script - Using Cloudflare Official Client
# Automatically routes Google traffic through WARP to unlock restricted services

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
CYAN='\033[0;36m'
NC='\033[0m'

# Display Banner
show_banner() {
    clear
    echo -e "${CYAN}"
    echo "╔════════════════════════════════════════════════════╗"
    echo "║     🌐 WARP One-Click - Google Auto Unlock 🌐       ║"
    echo "║         Using Cloudflare Official Client           ║"
    echo "╚════════════════════════════════════════════════════╝"
    echo -e "${NC}"
}

# Check for root privileges
[[ $EUID -ne 0 ]] && { echo -e "${RED}Please run as root!${NC}"; exit 1; }

# System Detection
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$ID
    VERSION=$VERSION_ID
    CODENAME=$VERSION_CODENAME
else
    echo -e "${RED}Unable to detect system version${NC}"; exit 1
fi

ARCH=$(dpkg --print-architecture 2>/dev/null || echo "amd64")
echo -e "${GREEN}System: $OS $VERSION ($CODENAME) $ARCH${NC}"

# Display Current IP Information
echo -e "\n${YELLOW}Current IP Info:${NC}"
CURRENT_IP=$(curl -4 -s --max-time 5 ip.sb)
IP_INFO=$(curl -s --max-time 5 "http://ip-api.com/json/$CURRENT_IP" 2>/dev/null)
echo -e "IP: ${GREEN}$CURRENT_IP${NC}"
echo -e "Location: ${GREEN}$(echo $IP_INFO | grep -oP '"country":"\K[^"]+') - $(echo $IP_INFO | grep -oP '"city":"\K[^"]+')${NC}"

# Install Cloudflare WARP Official Client
install_warp() {
    echo -e "\n${CYAN}[1/3] Installing Cloudflare WARP Client...${NC}"
    
    case $OS in
        ubuntu|debian)
            # Install necessary dependencies
            apt-get update -y >/dev/null 2>&1
            apt-get install -y gnupg curl wget lsb-release >/dev/null 2>&1
            
            # Add Cloudflare GPG key
            curl -fsSL https://pkg.cloudflareclient.com/pubkey.gpg | gpg --yes --dearmor --output /usr/share/keyrings/cloudflare-warp-archive-keyring.gpg
            
            # Add repository
            echo "deb [arch=$ARCH signed-by=/usr/share/keyrings/cloudflare-warp-archive-keyring.gpg] https://pkg.cloudflareclient.com/ $CODENAME main" > /etc/apt/sources.list.d/cloudflare-client.list
            
            # Install
            apt-get update -y
            apt-get install -y cloudflare-warp
            ;;
        centos|rhel|rocky|almalinux|fedora)
            # Add repository
            cat > /etc/yum.repos.d/cloudflare-warp.repo << 'EOF'
[cloudflare-warp]
name=Cloudflare WARP
baseurl=https://pkg.cloudflareclient.com/rpm
enabled=1
gpgcheck=1
gpgkey=https://pkg.cloudflareclient.com/pubkey.gpg
EOF
            if command -v dnf &>/dev/null; then
                dnf install -y cloudflare-warp
            else
                yum install -y cloudflare-warp
            fi
            ;;
        *)
            echo -e "${RED}Unsupported system: $OS${NC}"
            echo -e "${YELLOW}Supported systems: Ubuntu, Debian, CentOS, RHEL, Rocky, AlmaLinux, Fedora${NC}"
            exit 1
            ;;
    esac
    
    if ! command -v warp-cli &>/dev/null; then
        echo -e "${RED}WARP installation failed${NC}"
        exit 1
    fi
    
    echo -e "${GREEN}✓ WARP client installed successfully${NC}"
}

# Configure WARP
configure_warp() {
    echo -e "\n${CYAN}[2/3] Configuring WARP...${NC}"
    
    # Register device
    echo -e "Registering device..."
    warp-cli --accept-tos registration new 2>/dev/null || warp-cli --accept-tos register 2>/dev/null || true
    
    # Set to proxy mode (won't take over all traffic, only via SOCKS5)
    warp-cli --accept-tos mode proxy 2>/dev/null || warp-cli mode proxy 2>/dev/null || true
    
    # Set proxy port
    warp-cli --accept-tos proxy port 40000 2>/dev/null || warp-cli proxy port 40000 2>/dev/null || true
    
    # Connect
    echo -e "Connecting to WARP..."
    warp-cli --accept-tos connect 2>/dev/null || warp-cli connect 2>/dev/null
    
    sleep 3
    
    # Display Status
    STATUS=$(warp-cli --accept-tos status 2>/dev/null || warp-cli status 2>/dev/null)
    echo -e "Status: ${GREEN}$STATUS${NC}"
    
    echo -e "${GREEN}✓ WARP configuration complete${NC}"
}

# Setup Transparent Proxy (Auto-route Google traffic)
setup_transparent_proxy() {
    echo -e "\n${CYAN}[3/3] Setting up transparent proxy rules...${NC}"
    
    # Disable IPv6 access to Google (prevents detection due to IPv4/v6 mismatch)
    echo -e "Configuring IPv6 rules..."
    
    # Method 1: Add IPv6 blackhole route for Google IP range
    # Google IPv6 range: 2607:f8b0::/32
    ip -6 route add blackhole 2607:f8b0::/32 2>/dev/null || true
    
    # Method 2: Set system preference to IPv4
    if ! grep -q "precedence ::ffff:0:0/96  100" /etc/gai.conf 2>/dev/null; then
        echo "precedence ::ffff:0:0/96  100" >> /etc/gai.conf
    fi
    
    # Install redsocks (Transparent proxy tool)
    case $OS in
        ubuntu|debian)
            apt-get install -y redsocks iptables >/dev/null 2>&1
            ;;
        centos|rhel|rocky|almalinux|fedora)
            if command -v dnf &>/dev/null; then
                dnf install -y redsocks iptables >/dev/null 2>&1
            else
                yum install -y redsocks iptables >/dev/null 2>&1
            fi
            ;;
    esac
    
    # Create redsocks configuration
    cat > /etc/redsocks.conf << 'EOF'
base {
    log_debug = off;
    log_info = on;
    log = "syslog:daemon";
    daemon = on;
    redirector = iptables;
}

redsocks {
    local_ip = 127.0.0.1;
    local_port = 12345;
    ip = 127.0.0.1;
    port = 40000;
    type = socks5;
}
EOF

    # Create iptables rule script
    cat > /usr/local/bin/warp-google << 'SCRIPT'
#!/bin/bash

# Google IP Ranges
GOOGLE_IPS="
8.8.4.0/24
8.8.8.0/24
34.0.0.0/9
35.184.0.0/13
35.192.0.0/12
35.224.0.0/12
35.240.0.0/13
64.233.160.0/19
66.102.0.0/20
66.249.64.0/19
72.14.192.0/18
74.125.0.0/16
104.132.0.0/14
108.177.0.0/17
142.250.0.0/15
172.217.0.0/16
172.253.0.0/16
173.194.0.0/16
209.85.128.0/17
216.58.192.0/19
216.239.32.0/19
"

start() {
    echo "Starting Google transparent proxy..."
    
    # Start redsocks
    pkill redsocks 2>/dev/null
    redsocks -c /etc/redsocks.conf
    
    # Create new iptables chain
    iptables -t nat -N WARP_GOOGLE 2>/dev/null || iptables -t nat -F WARP_GOOGLE
    
    # Add Google IP rules
    for ip in $GOOGLE_IPS; do
        iptables -t nat -A WARP_GOOGLE -d $ip -p tcp -j REDIRECT --to-ports 12345
    done
    
    # Apply to OUTPUT chain
    iptables -t nat -C OUTPUT -j WARP_GOOGLE 2>/dev/null || iptables -t nat -A OUTPUT -j WARP_GOOGLE
    
    echo "Google transparent proxy started"
}

stop() {
    echo "Stopping Google transparent proxy..."
    pkill redsocks 2>/dev/null
    iptables -t nat -D OUTPUT -j WARP_GOOGLE 2>/dev/null
    iptables -t nat -F WARP_GOOGLE 2>/dev/null
    iptables -t nat -X WARP_GOOGLE 2>/dev/null
    echo "Google transparent proxy stopped"
}

status() {
    echo "=== WARP Status ==="
    warp-cli status 2>/dev/null || echo "WARP not running"
    echo ""
    echo "=== Redsocks Status ==="
    pgrep -x redsocks >/dev/null && echo "Running" || echo "Not running"
    echo ""
    echo "=== iptables Rules ==="
    iptables -t nat -L WARP_GOOGLE -n 2>/dev/null | head -5 || echo "No rules found"
}

case "$1" in
    start) start ;;
    stop) stop ;;
    restart) stop; sleep 1; start ;;
    status) status ;;
    *) echo "Usage: $0 {start|stop|restart|status}" ;;
esac
SCRIPT

    chmod +x /usr/local/bin/warp-google
    
    # Start transparent proxy
    /usr/local/bin/warp-google start
    
    # Create systemd service
    cat > /etc/systemd/system/warp-google.service << 'EOF'
[Unit]
Description=WARP Google Transparent Proxy
After=network.target warp-svc.service

[Service]
Type=oneshot
RemainAfterExit=yes
ExecStart=/usr/local/bin/warp-google start
ExecStop=/usr/local/bin/warp-google stop

[Install]
WantedBy=multi-user.target
EOF

    systemctl daemon-reload
    systemctl enable warp-google 2>/dev/null
    
    echo -e "${GREEN}✓ Transparent proxy configuration complete${NC}"
}

# Test Connection
test_connection() {
    echo -e "\n${CYAN}Testing connection...${NC}"
    
    sleep 2
    
    # Test Google
    GOOGLE_TEST=$(curl -s --max-time 10 -o /dev/null -w "%{http_code}" https://www.google.com)
    if [ "$GOOGLE_TEST" = "200" ]; then
        echo -e "${GREEN}✓ Google connection successful!${NC}"
    else
        echo -e "${YELLOW}Google test returned: $GOOGLE_TEST${NC}"
    fi
    
    # Show WARP IP
    WARP_IP=$(curl -x socks5://127.0.0.1:40000 -s --max-time 10 ip.sb 2>/dev/null)
    if [ -n "$WARP_IP" ]; then
        WARP_INFO=$(curl -s --max-time 5 "http://ip-api.com/json/$WARP_IP" 2>/dev/null)
        echo -e "\nWARP IP: ${GREEN}$WARP_IP${NC}"
        echo -e "WARP Location: ${GREEN}$(echo $WARP_INFO | grep -oP '"country":"\K[^"]+') - $(echo $WARP_INFO | grep -oP '"city":"\K[^"]+')${NC}"
    fi
}

# Create Management Script
create_management() {
    cat > /usr/local/bin/warp << 'EOF'
#!/bin/bash
case "$1" in
    status)
        warp-cli status 2>/dev/null
        echo ""
        /usr/local/bin/warp-google status 2>/dev/null
        ;;
    start)
        warp-cli connect 2>/dev/null
        /usr/local/bin/warp-google start
        ;;
    stop)
        /usr/local/bin/warp-google stop
        warp-cli disconnect 2>/dev/null
        ;;
    restart)
        $0 stop
        sleep 2
        $0 start
        ;;
    test)
        echo "Testing Google connection..."
        curl -s --max-time 10 -o /dev/null -w "Status Code: %{http_code}\n" https://www.google.com
        ;;
    ip)
        echo "Direct IP:"
        curl -4 -s ip.sb
        echo ""
        echo "WARP IP:"
        curl -x socks5://127.0.0.1:40000 -s ip.sb
        echo ""
        ;;
    uninstall)
        echo "Uninstalling..."
        /usr/local/bin/warp-google stop 2>/dev/null
        warp-cli disconnect 2>/dev/null
        systemctl disable warp-google 2>/dev/null
        rm -f /etc/systemd/system/warp-google.service
        rm -f /usr/local/bin/warp-google
        rm -f /usr/local/bin/warp
        rm -f /etc/redsocks.conf
        apt-get remove -y cloudflare-warp redsocks 2>/dev/null || yum remove -y cloudflare-warp redsocks 2>/dev/null
        echo "WARP uninstalled"
        ;;
    *)
        echo "WARP Management Tool"
        echo ""
        echo "Usage: warp <command>"
        echo ""
        echo "Commands:"
        echo "  status    Check status"
        echo "  start     Start WARP"
        echo "  stop      Stop WARP"
        echo "  restart   Restart WARP"
        echo "  test      Test Google connection"
        echo "  ip        Show IP info"
        echo "  uninstall Uninstall WARP"
        ;;
esac
EOF
    chmod +x /usr/local/bin/warp
}

# Main Installation Process
do_install() {
    install_warp
    configure_warp
    setup_transparent_proxy
    create_management
    test_connection
    
    echo -e "\n${GREEN}╔════════════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║          🎉 Install Finished! Google Unlocked 🎉    ║${NC}"
    echo -e "${GREEN}╚════════════════════════════════════════════════════╝${NC}"
    echo -e "\n${YELLOW}All Google traffic is now automatically routed via WARP!${NC}"
    echo -e "${YELLOW}No extra configuration needed; just access services directly.${NC}"
    echo -e "\nManagement Command: ${CYAN}warp {status|start|stop|restart|test|ip|uninstall}${NC}\n"
}

# Uninstallation
do_uninstall() {
    echo -e "\n${YELLOW}Uninstalling WARP...${NC}"
    /usr/local/bin/warp-google stop 2>/dev/null
    warp-cli disconnect 2>/dev/null
    systemctl disable warp-google 2>/dev/null
    systemctl stop warp-svc 2>/dev/null
    rm -f /etc/systemd/system/warp-google.service
    rm -f /usr/local/bin/warp-google
    rm -f /usr/local/bin/warp
    rm -f /etc/redsocks.conf
    
    # Clean up iptables rules
    iptables -t nat -D OUTPUT -j WARP_GOOGLE 2>/dev/null
    iptables -t nat -F WARP_GOOGLE 2>/dev/null
    iptables -t nat -X WARP_GOOGLE 2>/dev/null
    
    # Remove IPv6 blackhole route
    ip -6 route del blackhole 2607:f8b0::/32 2>/dev/null
    
    # Remove packages
    case $OS in
        ubuntu|debian)
            apt-get remove -y cloudflare-warp redsocks 2>/dev/null
            rm -f /etc/apt/sources.list.d/cloudflare-client.list
            ;;
        centos|rhel|rocky|almalinux|fedora)
            yum remove -y cloudflare-warp redsocks 2>/dev/null || dnf remove -y cloudflare-warp redsocks 2>/dev/null
            rm -f /etc/yum.repos.d/cloudflare-warp.repo
            ;;
    esac
    
    echo -e "${GREEN}✓ WARP has been completely uninstalled${NC}\n"
}

# Check Status
do_status() {
    echo -e "\n${CYAN}══════════════ WARP Running Status ══════════════${NC}\n"
    
    # WARP Client Status
    echo -e "${YELLOW}[WARP Client]${NC}"
    if command -v warp-cli &>/dev/null; then
        warp-cli status 2>/dev/null || echo "Not running"
    else
        echo -e "${RED}Not installed${NC}"
    fi
    
    echo ""
    
    # Redsocks Status
    echo -e "${YELLOW}[Transparent Proxy]${NC}"
    if pgrep -x redsocks >/dev/null; then
        echo -e "${GREEN}Running${NC}"
    else
        echo -e "${RED}Not running${NC}"
    fi
    
    echo ""
    
    # iptables Rules
    echo -e "${YELLOW}[iptables Rules]${NC}"
    iptables -t nat -L WARP_GOOGLE -n 2>/dev/null | head -3 || echo -e "${RED}No rules found${NC}"
    
    echo -e "\n${CYAN}════════════════════════════════════════════════${NC}\n"
}

# Show IP Info
do_show_ip() {
    echo -e "\n${CYAN}══════════════ IP Information ══════════════${NC}\n"
    
    echo -e "${YELLOW}[Direct IP]${NC}"
    DIRECT_IP=$(curl -4 -s --max-time 5 ip.sb)
    DIRECT_INFO=$(curl -s --max-time 5 "http://ip-api.com/json/$DIRECT_IP" 2>/dev/null)
    echo -e "IP: ${GREEN}$DIRECT_IP${NC}"
    echo -e "Location: $(echo $DIRECT_INFO | grep -oP '"country":"\K[^"]+') - $(echo $DIRECT_INFO | grep -oP '"city":"\K[^"]+')\n"
    
    echo -e "${YELLOW}[WARP IP]${NC}"
    WARP_IP=$(curl -x socks5://127.0.0.1:40000 -s --max-time 5 ip.sb 2>/dev/null)
    if [ -n "$WARP_IP" ]; then
        WARP_INFO=$(curl -s --max-time 5 "http://ip-api.com/json/$WARP_IP" 2>/dev/null)
        echo -e "IP: ${GREEN}$WARP_IP${NC}"
        echo -e "Location: $(echo $WARP_INFO | grep -oP '"country":"\K[^"]+') - $(echo $WARP_INFO | grep -oP '"city":"\K[^"]+')\n"
    else
        echo -e "${RED}Unable to fetch (WARP might not be running)${NC}\n"
    fi
    
    echo -e "${CYAN}═════════════════════════════════════════════${NC}\n"
}

# Test Google Connection
do_test_google() {
    echo -e "\n${CYAN}Testing Google connection...${NC}"
    RESULT=$(curl -s --max-time 10 -o /dev/null -w "%{http_code}" https://www.google.com)
    if [ "$RESULT" = "200" ]; then
        echo -e "${GREEN}✓ Google connected successfully! Status: $RESULT${NC}\n"
    else
        echo -e "${RED}✗ Google connection failed, Status: $RESULT${NC}\n"
    fi
}

# Start Services
do_start() {
    echo -e "\n${CYAN}Starting WARP services...${NC}"
    warp-cli connect 2>/dev/null
    /usr/local/bin/warp-google start 2>/dev/null
    echo -e "${GREEN}✓ WARP started${NC}\n"
}

# Stop Services
do_stop() {
    echo -e "\n${CYAN}Stopping WARP services...${NC}"
    /usr/local/bin/warp-google stop 2>/dev/null
    warp-cli disconnect 2>/dev/null
    echo -e "${GREEN}✓ WARP stopped${NC}\n"
}

# Show Menu
show_menu() {
    echo -e "${YELLOW}Please select an option:${NC}\n"
    echo -e "  ${GREEN}1.${NC} Install WARP (Unlock Gemini, Play Store, etc.)"
    echo -e "  ${GREEN}2.${NC} Uninstall WARP"
    echo -e "  ${GREEN}3.${NC} Check Status"
    echo -e "  ${GREEN}0.${NC} Exit\n"
    
    read -p "Enter choice [0-3]: " choice
    
    case $choice in
        1) do_install ;;
        2) do_uninstall ;;
        3) do_status; do_show_ip; do_test_google ;;
        0) echo -e "\n${GREEN}Goodbye!${NC}\n"; exit 0 ;;
        *) echo -e "\n${RED}Invalid option${NC}\n" ;;
    esac
}

# Main Entry Point
main() {
    show_banner
    
    # Check root
    [[ $EUID -ne 0 ]] && { echo -e "${RED}Please run as root!${NC}"; exit 1; }
    
    # Detect System
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        OS=$ID
        VERSION=$VERSION_ID
        CODENAME=$VERSION_CODENAME
    else
        echo -e "${RED}Unable to detect system${NC}"; exit 1
    fi
    
    ARCH=$(dpkg --print-architecture 2>/dev/null || echo "amd64")
    echo -e "${GREEN}System: $OS $VERSION ($CODENAME) $ARCH${NC}\n"
    
    show_menu
}

main
