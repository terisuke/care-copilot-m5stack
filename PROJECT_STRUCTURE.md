# Project Structure

## Directory Overview

```
care-copilot-m5stack/
├── .github/                    # GitHub specific files
│   └── workflows/              # GitHub Actions CI/CD workflows
│       └── ci.yml             # Continuous Integration workflow
│
├── 01-contest-mvp/            # MVP development for M5Stack Contest
│   ├── README.md              # 6-day development plan
│   ├── hardware-list.md      # Required hardware components
│   ├── development-plan.md   # Detailed implementation schedule
│   └── demo-scenario.md      # Demo presentation scenario
│
├── 02-commercialization/      # Business and commercialization strategy
│   ├── business-plan.md      # Business model and revenue projections
│   ├── market-analysis.md    # Target market and competitor analysis
│   └── roadmap.md            # Product development roadmap
│
├── 03-technical-social-impact/ # Technical specifications and impact
│   ├── technical-spec.md     # Detailed technical documentation
│   ├── social-impact.md      # Social benefits and impact assessment
│   └── competitors.md        # Competitive analysis
│
├── 04-implementation/         # Core implementation files
│   ├── backend-server.js     # Node.js backend server
│   ├── m5stack-firmware.ino  # Arduino firmware for M5Stack
│   └── config-template.h     # Configuration template for firmware
│
├── 05-investor-materials/     # Investment and presentation materials
│   └── pitch-deck.md         # Investor pitch deck
│
├── node_modules/              # Node.js dependencies (git-ignored)
├── venv/                      # Python virtual environment (git-ignored)
│
├── .env.example               # Environment variables template
├── .gitignore                 # Git ignore file
├── ACTION_PLAN.md            # Development action plan
├── CLAUDE.md                 # Claude Code assistant instructions
├── CONTRIBUTING.md           # Contribution guidelines
├── HARDWARE_UPDATE.md        # Hardware update notes
├── LICENSE                   # MIT License
├── package.json              # Node.js project configuration
├── package-lock.json         # Node.js dependency lock file
├── PROJECT_STRUCTURE.md      # This file
└── README.md                 # Main project documentation
```

## Key Files

### Configuration Files
- `.env.example` - Template for environment variables
- `config-template.h` - Template for Arduino firmware configuration
- `package.json` - Node.js dependencies and scripts

### Documentation
- `README.md` - Project overview and setup instructions
- `CLAUDE.md` - AI assistant context and instructions
- `CONTRIBUTING.md` - Guidelines for contributors

### Implementation
- `backend-server.js` - Main backend server handling MQTT, WebSocket, and REST API
- `m5stack-firmware.ino` - ESP32 firmware for M5Stack devices

### Business Documents
- Contest submission materials in `01-contest-mvp/`
- Business planning in `02-commercialization/`
- Technical documentation in `03-technical-social-impact/`

## Development Workflow

1. **Hardware Setup**: Configure M5Stack devices with firmware
2. **Backend Setup**: Deploy Node.js server with PostgreSQL and Redis
3. **Integration**: Connect devices via MQTT to backend
4. **Monitoring**: Use web dashboard for real-time monitoring
5. **Alerts**: Configure LINE Bot for notifications

## Technology Stack

- **Hardware**: M5Stack Core2, M5StickC Plus2, various sensors
- **Firmware**: Arduino/C++ on ESP32
- **Backend**: Node.js, Express, Socket.io
- **Database**: PostgreSQL, Redis
- **Communication**: MQTT, WebSocket, REST API
- **Notifications**: LINE Messaging API