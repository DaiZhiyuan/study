# Intel performance tools

Server is designed for Intel Processors and as such many of the tools published by Intel are helpful on both Linux and Windows:

- Memory Latency Checker (MLC) — https://www.intel.com/content/www/us/en/developer/articles/tool/intelr-memory-latency-checker.html
- Power/Thermal Utility (PTU) — https://designintools.intel.com/product_p/stlgrn28.htm
- Performance Counter Monitor (PCM) — https://designintools.intel.com/Performance_Counter_Monitor_PCM_p/stlgrn36.htm
  - pcm-memory.x — provides read and write details per channel.
  - Intel upgrades the PCM suite regularly to include useful tools including:
    - pcm-power — provides frequency, UPI low power modes, along with P and C-state residency statistics.
    - pcm-memory — provides memory read and write details per channel.
    - pcm-iio — provides I/O statistics.
