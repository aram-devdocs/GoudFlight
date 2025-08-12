#!/usr/bin/env python3
"""Test footer rendering"""

from rich.console import Console
from rich.panel import Panel
from rich.text import Text

console = Console()

# Test footer creation
def create_footer(width):
    if width < 60:
        help_str = "Q:Quit Tab:Switch ↑↓:Scroll"
    elif width < 100:
        help_str = "Q: Quit | Tab: Focus | ↑↓: Scroll | S/T/R: Commands"
    else:
        help_str = "Tab: Focus | ↑↓: Scroll | Q: Quit | C: Clear | S/T/R: Commands"
    
    help_text = Text(help_str, style="bold white")
    
    return Panel(
        help_text,
        style="white on grey23",
        border_style="bright_blue",
        padding=(0, 1)
    )

# Test rendering
width = console.size.width
footer = create_footer(width)

print(f"Terminal width: {width}")
print("Footer content:")
console.print(footer)
print("\nIf you see an empty box above, there's a rendering issue.")