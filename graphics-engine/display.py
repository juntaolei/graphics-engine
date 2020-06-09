from os import (execlp, fork, remove)
from subprocess import (PIPE, Popen)

from .constants import (XRES, YRES, MAX_COLOR, DEFAULT_COLOR)


def new_screen(width: int = XRES, height: int = YRES) -> list:
    """Create a new screen as represented by a multi-dimensional list."""
    screen = []
    for y in range(height):
        screen.append([DEFAULT_COLOR for x in range(width)])

    return screen


def new_zbuffer(width: int = XRES, height: int = YRES) -> list:
    """Create a new z-buffer as represented by a multi-dimensional list."""
    zbuffer = []
    for y in range(height):
        zbuffer.append([float("-inf") for x in range(width)])

    return zbuffer


def plot(screen: list, zbuffer: list, color: list, x: int, y: int,
         z: float) -> None:
    """Plot a point on the given screen and z-buffer with the given color."""
    new_y = YRES - 1 - y
    z = int(z * 1000) / 1000.0

    if x >= 0 and x < XRES and new_y >= 0 and new_y < YRES and zbuffer[new_y][
            x] <= z:
        screen[new_y][x] = color
        zbuffer[new_y][x] = z


def clear_screen(screen: list) -> None:
    """Fill an existing list with the default color."""
    for y in range(len(screen)):
        for x in range(len(screen[y])):
            screen[y][x] = DEFAULT_COLOR


def clear_zbuffer(zbuffer: list) -> None:
    """Fill an existing list with negative infinity."""
    for y in range(len(zbuffer)):
        for x in range(len(zbuffer[y])):
            zbuffer[y][x] = float("-inf")


def save_ppm(screen: list, filename: str) -> None:
    """Save a screen to a PPM image with the given filename."""
    with open(filename, mode="wb") as f:
        f.write(
            str.encode(f"P3\n{len(screen[0])} {len(screen)} {MAX_COLOR}\n"))
        for y in range(len(screen)):
            for x in range(len(screen[y])):
                f.write(bytes(screen[y][x]))


def save_extension(screen: list, filename: str) -> None:
    """Save a screen to an image with the given filename and extension."""
    ppm_name = f"{filename[:filename.find('.')]}.ppm"
    save_ppm(screen, ppm_name)
    Popen(["convert", ppm_name, filename], stdin=PIPE,
          stdout=PIPE).communicate()
    remove(ppm_name)


def display(screen: list) -> None:
    """Display a given screen."""
    save_ppm(screen, "pic.ppm")
    Popen(["display", "pic.ppm"], stdin=PIPE, stdout=PIPE).communicate()
    remove("pic.ppm")


def make_animation(name: str) -> None:
    """Make an animation with the given name."""
    name, name_arg = f"{name}.gif", f"anim/{name}*"
    print(f"Saving animation as {name}")
    if fork() == 0:
        execlp("convert", "convert", "-delay", "1.7", name_arg, name)
