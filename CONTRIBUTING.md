# Contributing to Care Copilot

Thank you for your interest in contributing to the Care Copilot project! This document provides guidelines for contributing to the project.

## Getting Started

1. Fork the repository
2. Clone your fork: `git clone https://github.com/terisuke/care-copilot-m5stack.git`
3. Create a feature branch: `git checkout -b feature/your-feature-name`
4. Make your changes
5. Commit your changes: `git commit -m 'Add some feature'`
6. Push to the branch: `git push origin feature/your-feature-name`
7. Open a Pull Request

## Development Setup

### Prerequisites
- Arduino IDE 2.0+ with ESP32 board support
- Node.js 18+ and npm
- Python 3.9+ (for AI/analytics components)
- PostgreSQL 14+
- Redis 6+

### Environment Setup
1. Copy `.env.example` to `.env` and configure your environment variables
2. Install Node.js dependencies: `npm install`
3. Set up the Arduino IDE with M5Stack libraries

## Code Style Guidelines

### JavaScript (Node.js)
- Use ES6+ features
- Follow standard JavaScript conventions
- Use async/await for asynchronous operations
- Add JSDoc comments for functions

### Arduino/C++
- Follow Arduino coding conventions
- Keep functions small and focused
- Comment complex logic
- Use meaningful variable names

### Python
- Follow PEP 8 style guide
- Use type hints where appropriate
- Document functions with docstrings

## Testing

- Test your changes locally before submitting a PR
- Include test cases for new features
- Ensure existing tests pass
- Test with actual M5Stack hardware if possible

## Pull Request Process

1. Update the README.md with details of changes if applicable
2. Update documentation for API changes
3. Ensure your code follows the project's style guidelines
4. Your PR will be reviewed by maintainers

## Reporting Issues

When reporting issues, please include:
- Description of the issue
- Steps to reproduce
- Expected behavior
- Actual behavior
- Hardware/software environment details
- Relevant logs or error messages

## Feature Requests

We welcome feature requests! Please:
- Check existing issues first
- Clearly describe the feature
- Explain the use case
- Consider submitting a PR if you can implement it

## Questions?

Feel free to open an issue for any questions about contributing.

## License

By contributing, you agree that your contributions will be licensed under the MIT License.