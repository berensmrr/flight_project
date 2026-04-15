# Flight Passenger Log Conversion Tool

A command-line tool to convert passenger logs between CSV, Binary, and XML formats.

## Requirements

```bash
sudo apt-get install libxml2-dev
```

## Compilation

```bash
gcc flightTool.c -o flightTool -lxml2 -I/usr/include/libxml2
```

## Usage

```bash
./flightTool <input_file> <output_file> <conversion_type> -separator <1|2|3> -opsys <1|2|3> [-encoding <1|2|3>]
```

### Conversion Types
- `1` → CSV to Binary
- `2` → Binary to XML
- `3` → XSD Validation
- `4` → XML Encoding Conversion

### Arguments
- `-separator` → 1=comma, 2=tab, 3=semicolon
- `-opsys` → 1=windows, 2=linux, 3=macos
- `-encoding` → 1=UTF-16LE, 2=UTF-16BE, 3=UTF-8

## Examples

```bash
# CSV to Binary
./flightTool flightlog.csv flightdata.dat 1 -separator 1 -opsys 2

# Binary to XML
./flightTool flightdata.dat flightlogs.xml 2 -separator 1 -opsys 2

# XSD Validation
./flightTool flightlogs.xml flightlogs.xsd 3 -separator 1 -opsys 2

# Convert to UTF-16LE
./flightTool flightlogs.xml flightlogs_utf16le.xml 4 -separator 1 -opsys 2 -encoding 1

# Convert to UTF-16BE
./flightTool flightlogs.xml flightlogs_utf16be.xml 4 -separator 1 -opsys 2 -encoding 2

# Convert back to UTF-8
./flightTool flightlogs_utf16le.xml flightlogs_back.xml 4 -separator 1 -opsys 2 -encoding 3
```

## Data Dictionary

| Field | Description |
|-------|-------------|
| ticket_id | 3 uppercase letters + 4 digits (e.g. THY1234) |
| timestamp | ISO 8601 format (e.g. 2025-03-01T14:25:00) |
| baggage_weight | Float, range 0.0 - 50.0 kg |
| loyalty_points | Integer, range 0 - 10000 |
| status | 🟢 BOARDED, 🔴 CANCELLED, ⚠️ DELAYED |
| destination | Max 30 characters |
| cabin_class | ECONOMY, BUSINESS, FIRST |
| seat_num | Integer, range 1 - 300 |
| app_ver | Version string (e.g. v1.0.2) |
| passenger_name | Full name, UTF-8 formatted |
