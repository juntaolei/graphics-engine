from math import sqrt

from .constants import (AMBIENT, DIFFUSE, SPECULAR, LOCATION, COLOR,
                        SPECULAR_EXP, RED, GREEN, BLUE)


def dot_product(a, b) -> float:
    """Calculate the dot product of two vectors."""
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]


def normalize(vector: list) -> None:
    """Normalize the given vector."""
    magnitude = sqrt(vector[0] * vector[0] + vector[1] * vector[1] +
                     vector[2] * vector[2])
    for i in range(3):
        vector[i] = vector[i] / magnitude


def calculate_normal(polygons: list, i: int) -> list:
    """Calculate the normal values for polygons."""
    A, B, N = [0, 0, 0], [0, 0, 0], [0, 0, 0]

    A[0] = polygons[i + 1][0] - polygons[i][0]
    A[1] = polygons[i + 1][1] - polygons[i][1]
    A[2] = polygons[i + 1][2] - polygons[i][2]

    B[0] = polygons[i + 2][0] - polygons[i][0]
    B[1] = polygons[i + 2][1] - polygons[i][1]
    B[2] = polygons[i + 2][2] - polygons[i][2]

    N[0] = A[1] * B[2] - A[2] * B[1]
    N[1] = A[2] * B[0] - A[0] * B[2]
    N[2] = A[0] * B[1] - A[1] * B[0]

    return N


def calculate_ambient(alight: list, reflect: dict) -> list:
    """Calculate the light ambient values."""
    a = [0, 0, 0]
    a[RED] = alight[RED] * reflect['red'][AMBIENT]
    a[GREEN] = alight[GREEN] * reflect['green'][AMBIENT]
    a[BLUE] = alight[BLUE] * reflect['blue'][AMBIENT]
    return a


def calculate_diffuse(light: list, reflect: dict, normal: list) -> list:
    """Calculate the light diffuse values."""
    d = [0, 0, 0]

    dot = dot_product(light[LOCATION], normal)
    dot = dot if dot > 0 else 0

    d[RED] = light[COLOR][RED] * reflect['red'][DIFFUSE] * dot
    d[GREEN] = light[COLOR][GREEN] * reflect['green'][DIFFUSE] * dot
    d[BLUE] = light[COLOR][BLUE] * reflect['blue'][DIFFUSE] * dot
    return d


def calculate_specular(light: list, reflect: dict, view: list,
                       normal: list) -> list:
    """Calculate the specular values."""
    s, n = [0, 0, 0], [0, 0, 0]

    result = 2 * dot_product(light[LOCATION], normal)
    n[0] = (normal[0] * result) - light[LOCATION][0]
    n[1] = (normal[1] * result) - light[LOCATION][1]
    n[2] = (normal[2] * result) - light[LOCATION][2]

    result = dot_product(n, view)
    result = result if result > 0 else 0
    result = pow(result, SPECULAR_EXP)

    s[RED] = light[COLOR][RED] * reflect['red'][SPECULAR] * result
    s[GREEN] = light[COLOR][GREEN] * reflect['green'][SPECULAR] * result
    s[BLUE] = light[COLOR][BLUE] * reflect['blue'][SPECULAR] * result
    return s


def limit_color(color: list) -> None:
    """Limit the color range from 0 to 255 inclusive."""
    color[RED] = 255 if color[RED] > 255 else color[RED]
    color[GREEN] = 255 if color[GREEN] > 255 else color[GREEN]
    color[BLUE] = 255 if color[BLUE] > 255 else color[BLUE]


def get_lighting(normal: list, view: list, ambient: list, light: list,
                 symbols: dict, reflect: str) -> list:
    """Calculate the correct lighting values."""
    n = normal
    normalize(n)
    normalize(light[LOCATION])
    normalize(view)
    r = symbols[reflect][1]

    a = calculate_ambient(ambient, r)
    d = calculate_diffuse(light, r, n)
    s = calculate_specular(light, r, view, n)

    i = [0, 0, 0]
    i[RED] = int(a[RED] + d[RED] + s[RED])
    i[GREEN] = int(a[GREEN] + d[GREEN] + s[GREEN])
    i[BLUE] = int(a[BLUE] + d[BLUE] + s[BLUE])
    limit_color(i)

    return i
