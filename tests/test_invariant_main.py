import pytest
import ctypes
import struct


# Simulated safe buffer copy function that enforces bounds checking
# This represents what the secure implementation SHOULD do
SAFE_BUFFER_SIZE = 256  # Typical fixed-size stack buffer


def safe_strcpy(dest_size, src):
    """
    Simulates a bounds-checked string copy.
    Returns the copied string if it fits, raises ValueError if it would overflow.
    This is the SECURE version that should be used instead of strcpy.
    """
    if src is None:
        raise ValueError("Source string cannot be None")
    
    # Encode to bytes to simulate C string behavior
    if isinstance(src, str):
        src_bytes = src.encode('utf-8', errors='replace')
    else:
        src_bytes = src
    
    # +1 for null terminator, as in C
    if len(src_bytes) + 1 > dest_size:
        raise ValueError(
            f"Buffer overflow prevented: source length {len(src_bytes)} "
            f"exceeds destination buffer size {dest_size}"
        )
    
    return src_bytes[:dest_size - 1]  # Truncate with null terminator space


def vulnerable_strcpy_simulation(dest_size, src):
    """
    Simulates the VULNERABLE strcpy behavior from main.c:50.
    This does NOT check bounds — used to demonstrate what the vulnerable code does.
    Returns bytes that would be written, potentially exceeding dest_size.
    """
    if isinstance(src, str):
        src_bytes = src.encode('utf-8', errors='replace')
    else:
        src_bytes = src
    return src_bytes  # No bounds check — returns full source regardless of dest size


@pytest.mark.parametrize("payload", [
    # 2x buffer size overflow
    "A" * (SAFE_BUFFER_SIZE * 2),
    # 10x buffer size overflow
    "B" * (SAFE_BUFFER_SIZE * 10),
    # Exactly at boundary (should succeed)
    "C" * (SAFE_BUFFER_SIZE - 1),
    # One byte over boundary
    "D" * SAFE_BUFFER_SIZE,
    # Classic stack smashing pattern
    "A" * 260,
    # Return address overwrite pattern (PSP/Vita style)
    "\x41\x41\x41\x41" * 100,
    # Null bytes embedded in payload
    "A" * 100 + "\x00" + "B" * 200,
    # Format string mixed with overflow
    "%s%s%s%s%s%n" * 50,
    # Unicode/multibyte characters causing size mismatch
    "\u00ff" * 300,
    # Shell-like payload exceeding buffer
    "/bin/sh\x00" + "A" * 300,
    # Heap spray pattern
    "\x90" * 1024,  # NOP sled equivalent
    # PSP/Vita specific: MIPS shellcode-like pattern
    "\x27\xbd\xff\xe0\xaf\xbf\x00\x1c" * 50,
    # Empty string (edge case — should always be safe)
    "",
    # Single character (should always be safe)
    "X",
    # Exactly one byte under limit
    "E" * (SAFE_BUFFER_SIZE - 2),
    # Large repeated pattern
    "OVERFLOW" * 500,
    # Binary data pattern
    bytes(range(256)).decode('latin-1') * 4,
    # Whitespace flood
    " " * 2000,
    # Mixed attack payload
    "A" * 128 + "\x00\x00\x00\x00" + "\xff\xff\xff\xff" + "B" * 128,
])
def test_buffer_read_never_exceeds_declared_length(payload):
    """
    Invariant: Buffer reads/writes must NEVER exceed the declared buffer size.
    
    Any string copy operation must either:
    1. Reject/raise an error if the source exceeds destination buffer size, OR
    2. Truncate the source to fit within the destination buffer size.
    
    The vulnerable strcpy(msg, txt) at main.c:50 violates this invariant
    by blindly copying without bounds checking (CWE-120).
    """
    dest_size = SAFE_BUFFER_SIZE
    
    # Convert payload to bytes for length measurement
    if isinstance(payload, str):
        payload_bytes = payload.encode('utf-8', errors='replace')
    else:
        payload_bytes = payload
    
    payload_len = len(payload_bytes)
    
    # INVARIANT CHECK: The secure implementation must enforce bounds
    if payload_len + 1 > dest_size:
        # Oversized input MUST be rejected with an error
        with pytest.raises((ValueError, OverflowError, BufferError)):
            safe_strcpy(dest_size, payload)
        
        # Additionally verify: the vulnerable version WOULD overflow
        # (demonstrating the bug exists without bounds checking)
        vulnerable_result = vulnerable_strcpy_simulation(dest_size, payload)
        assert len(vulnerable_result) > dest_size - 1, (
            f"Expected vulnerable strcpy to write {len(vulnerable_result)} bytes "
            f"into a {dest_size}-byte buffer, confirming overflow would occur"
        )
    else:
        # Safe-sized input MUST succeed and fit within buffer
        result = safe_strcpy(dest_size, payload)
        
        # INVARIANT: Result must never exceed buffer size (minus null terminator)
        assert len(result) <= dest_size - 1, (
            f"Buffer overflow detected: copied {len(result)} bytes into "
            f"{dest_size}-byte buffer (max allowed: {dest_size - 1})"
        )
        
        # INVARIANT: Result must be a prefix of or equal to the original payload
        assert payload_bytes.startswith(result) or result == payload_bytes[:dest_size - 1], (
            "Copied data does not match expected truncated source"
        )


@pytest.mark.parametrize("payload,expected_safe", [
    ("A" * 255, True),   # Exactly fits (255 chars + null = 256)
    ("A" * 256, False),  # One byte too many
    ("A" * 512, False),  # 2x overflow
    ("A" * 2560, False), # 10x overflow
    ("", True),          # Empty string always safe
    ("X", True),         # Single char always safe
])
def test_boundary_conditions_for_buffer_size(payload, expected_safe):
    """
    Invariant: Boundary conditions at exact buffer size must be handled correctly.
    Strings of length == buffer_size - 1 must succeed.
    Strings of length >= buffer_size must be rejected.
    """
    dest_size = SAFE_BUFFER_SIZE
    
    if expected_safe:
        result = safe_strcpy(dest_size, payload)
        assert len(result) < dest_size, (
            f"Safe copy result {len(result)} must be less than buffer size {dest_size}"
        )
    else:
        with pytest.raises((ValueError, OverflowError, BufferError)):
            safe_strcpy(dest_size, payload)


def test_no_out_of_bounds_read_with_adversarial_inputs():
    """
    Invariant: Processing adversarial inputs must never result in reads
    beyond the declared buffer length, regardless of input content.
    """
    dest_size = SAFE_BUFFER_SIZE
    
    adversarial_inputs = [
        "A" * (dest_size * 2),
        "A" * (dest_size * 10),
        "\x00" * (dest_size * 5),
        "\xff" * (dest_size * 3),
        "%" * (dest_size * 4),
        "\n\r\t" * (dest_size * 2),
    ]
    
    for payload in adversarial_inputs:
        payload_bytes = payload.encode('utf-8', errors='replace')
        
        # INVARIANT: Must raise an error for oversized inputs
        with pytest.raises((ValueError, OverflowError, BufferError)):
            safe_strcpy(dest_size, payload)
        
        # INVARIANT: Vulnerable version confirms overflow would happen
        vulnerable_written = len(vulnerable_strcpy_simulation(dest_size, payload))
        assert vulnerable_written > dest_size - 1, (
            f"Payload of length {vulnerable_written} should overflow "
            f"buffer of size {dest_size}"
        )