## Disclaimer

This document provides the primary disclaimer text for this repository.
Translated README files may include short summaries for convenience, but this English version is the maintained reference text for repository-level usage, warranty, and liability boundaries.

## Intended Use

`Modbus-Tools` is provided for development, debugging, validation, education, and troubleshooting.
It is not designed, certified, or represented as a safety-critical, fail-safe, or regulatory-compliant control product.

## No Warranty

The software is provided on an `AS IS` basis, without warranties or conditions of any kind, express or implied, including merchantability, fitness for a particular purpose, non-infringement, accuracy, availability, or uninterrupted operation.

## Operational Risk

Users are responsible for verifying all behavior, protocol handling, and device interactions before using the software in any environment that could affect equipment, processes, data integrity, or human safety.

No claim is made that:

- protocol framing or parsing is error-free in all scenarios
- calculated values, decoded fields, or analyzer output are always correct
- retry, timeout, polling, logging, or update behavior matches every operational requirement

## Safety-Critical And Production Use

Do not rely on this software as the sole control, protection, or safety mechanism in industrial, medical, transportation, energy, building, or other safety-relevant systems.

If the software is used in production or near-production environments, that decision and its consequences remain the user's responsibility.

## Auto-Update And Network Behavior

The application may communicate with external services only for features explicitly triggered or enabled by the user, such as update checks or direct communication with target devices.

At the time of writing:

- built-in auto-update support is implemented only for Windows
- device and network communication behavior depends on user configuration and target environment

## Limitation Of Liability

To the maximum extent permitted by applicable law, the authors, contributors, and copyright holders are not liable for any direct, indirect, incidental, special, exemplary, or consequential damages arising from the use of, inability to use, or results of using this software.

This includes, without limitation, damage related to:

- device misconfiguration
- downtime or loss of production
- corrupted or lost data
- failed updates
- protocol misinterpretation
- hardware, network, or process-side side effects

## Third-Party Components

This repository depends on third-party components distributed under their own licenses.
Users are responsible for reviewing and complying with those licenses when building, distributing, or using the software.

## License

Nothing in this document replaces or overrides the terms of the repository's [MIT License](LICENSE).
