# Dear PyGui Initialization Error Fix

## Problem

When using this NES emulator library in a Python application with Dear PyGui, you may encounter the following error:

```
Exception: Error: [1000] Message:       width keyword does not exist.
SystemError: <built-in function add_text> returned a result with an exception set
```

This error occurs when using the `width` parameter with `dpg.add_text()`, which is not supported in newer versions of Dear PyGui.

## Root Cause

The error trace shows:
```python
File "E:\Coding\Oracle-of-Legends\src\gui\settings_window.py", line 281, in _build_panel_layout_configuration
    dpg.add_text("Top Left:", width=120)
```

The `dpg.add_text()` function does not accept a `width` keyword argument. This parameter was likely removed or never existed in the Dear PyGui API.

## Solution

### Option 1: Remove the width parameter (Recommended)

Simply remove the `width` parameter from all `dpg.add_text()` calls:

```python
# Before (incorrect):
dpg.add_text("Top Left:", width=120)

# After (correct):
dpg.add_text("Top Left:")
```

### Option 2: Use a table or group for alignment

If you need fixed-width alignment, use a table layout:

```python
with dpg.table(header_row=False):
    dpg.add_table_column(width_fixed=True, init_width_or_weight=120)
    dpg.add_table_column()
    
    with dpg.table_row():
        dpg.add_text("Top Left:")
        # Add other widgets here
```

### Option 3: Use horizontal layout with spacing

For simple alignment, use a group with horizontal layout:

```python
with dpg.group(horizontal=True):
    dpg.add_text("Top Left:")
    dpg.add_spacer(width=50)  # Adjust as needed
    # Add other widgets here
```

## Finding All Instances

To find all instances of this issue in your Python codebase:

```bash
# Search for all uses of width parameter with add_text
grep -r "dpg\.add_text.*width=" src/

# Or use ripgrep for better performance:
rg "dpg\.add_text.*width=" src/
```

## Implementation Steps

1. **Locate the file**: Find `settings_window.py` or the file mentioned in your error trace
2. **Find all occurrences**: Search for `dpg.add_text` calls with `width` parameter
3. **Remove the parameter**: Delete `width=<value>` from each call
4. **Test the application**: Run your application to ensure it works
5. **Adjust spacing if needed**: If alignment is critical, implement Option 2 or 3 above

## Example Fix for settings_window.py

If your `_build_panel_layout_configuration()` function looks like this:

```python
def _build_panel_layout_configuration():
    dpg.add_text("Top Left:", width=120)
    dpg.add_text("Top Right:", width=120)
    dpg.add_text("Bottom Left:", width=120)
    dpg.add_text("Bottom Right:", width=120)
```

Change it to:

```python
def _build_panel_layout_configuration():
    # Option 1: Simple removal
    dpg.add_text("Top Left:")
    dpg.add_text("Top Right:")
    dpg.add_text("Bottom Left:")
    dpg.add_text("Bottom Right:")
    
    # OR Option 2: With table for alignment
    LABEL_COLUMN_WIDTH = 120  # Use constant for maintainability
    
    with dpg.table(header_row=False):
        dpg.add_table_column(width_fixed=True, init_width_or_weight=LABEL_COLUMN_WIDTH)
        dpg.add_table_column()
        
        with dpg.table_row():
            dpg.add_text("Top Left:")
            # Add associated control here
        
        with dpg.table_row():
            dpg.add_text("Top Right:")
            # Add associated control here
        
        with dpg.table_row():
            dpg.add_text("Bottom Left:")
            # Add associated control here
        
        with dpg.table_row():
            dpg.add_text("Bottom Right:")
            # Add associated control here
```

## Additional Notes

- This issue is specific to Dear PyGui and does not affect the core nones-mod emulator library
- Always consult the [Dear PyGui documentation](https://dearpygui.readthedocs.io/) for current API parameters
- Test your changes thoroughly after applying the fix

## Related Issues

If you encounter similar "keyword does not exist" errors with other Dear PyGui widgets, apply the same approach:
1. Check the official Dear PyGui documentation for valid parameters
2. Remove any unsupported parameters
3. Use alternative layout methods if precise sizing is needed
