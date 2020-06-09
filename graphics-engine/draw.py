from math import (sin, cos, pi)

from display import plot
from gmath import (calculate_normal, get_lighting)
from matrix import generate_curve_coefs


def draw_line(x0: int, y0: int, z0: float, x1: int, y1: int, z1: float,
              screen: list, zbuffer: list, color: list) -> None:
    """Draw a line from one point to another using Bresenham's line algorithm."""
    if x0 > x1:
        xt, yt, zt = x0, y0, z0
        x0, y0, z0 = x1, y1, z1
        x1, y1, z1 = xt, yt, zt

    x, y, z = x0, y0, z0
    A, B = 2 * (y1 - y0), -2 * (x1 - x0)
    wide, tail = False, False

    if abs(x1 - x0) >= abs(y1 - y0):
        wide = True
        loop_start, loop_end = x, x1
        dx_east = dx_northeast = 1
        dy_east = 0
        d_east = A
        distance = x1 - x + 1
        if (A > 0):
            d = A + B / 2
            dy_northeast = 1
            d_northeast = A + B
        else:
            d = A - B / 2
            dy_northeast = -1
            d_northeast = A - B

    else:
        tall = True
        dx_east = 0
        dx_northeast = 1
        distance = abs(y1 - y) + 1
        if (A > 0):
            d = A / 2 + B
            dy_east = dy_northeast = 1
            d_northeast = A + B
            d_east = B
            loop_start = y
            loop_end = y1
        else:
            d = A / 2 - B
            dy_east = dy_northeast = -1
            d_northeast = A - B
            d_east = -1 * B
            loop_start = y1
            loop_end = y

    dz = (z1 - z0) / distance if distance != 0 else 0

    while loop_start < loop_end:
        plot(screen, zbuffer, color, x, y, z)
        if (wide and ((A > 0 and d > 0) or
                      (A < 0 and d < 0))) or (tall and ((A > 0 and d < 0) or
                                                        (A < 0 and d > 0))):
            x += dx_northeast
            y += dy_northeast
            d += d_northeast
        else:
            x += dx_east
            y += dy_east
            d += d_east

        loop_start += 1
        z += dz

    plot(screen, zbuffer, color, x, y, z)


def draw_lines(matrix: list, screen: list, zbuffer: list, color: list):
    """Draw lines on a given screen."""
    if len(matrix) < 2:
        print("Need at least 2 points to draw")
        return

    point = 0
    while point < len(matrix) - 1:
        draw_line(int(matrix[point][0]), int(matrix[point][1]),
                  matrix[point][2], int(matrix[point + 1][0]),
                  int(matrix[point + 1][1]), matrix[point + 1][2], screen,
                  zbuffer, color)
        point += 2


def add_point(matrix: list, x: float, y: float, z: float = 0.0) -> None:
    """Add a single point to a given matrix."""
    matrix.append([x, y, z, 1])


def add_edge(matrix: list, x0: float, y0: float, z0: float, x1: float,
             y1: float, z1: float) -> None:
    """Add an edge to a given matrix."""
    add_point(matrix, x0, y0, z0)
    add_point(matrix, x1, y1, z1)


def add_polygon(matrix: list, x0: float, y0: float, z0: float, x1: float,
                y1: float, z1: float, x2: float, y2: float, z2: float) -> None:
    """Add the points of a polygon to the given matrix."""
    add_point(matrix, x0, y0, z0)
    add_point(matrix, x1, y1, z1)
    add_point(matrix, x2, y2, z2)


def add_circle(matrix: list, cx: float, cy: float, cz: float, r: float,
               step: int) -> None:
    """Add the points in form of edges that form a circle."""
    x0, y0, i = r + cx, cy, 1

    while i <= step:
        t = float(i) / step
        x1, y1 = r * cos(2 * pi * t) + cx, r * sin(2 * pi * t) + cy

        add_edge(matrix, x0, y0, cz, x1, y1, cz)
        x0, y0 = x1, y1
        i += 1


def add_curve(matrix: list, x0: float, y0: float, x1: float, y1: float,
              x2: float, y2: float, x3: float, y3: float, step: int,
              curve_type: str) -> None:
    """
    Add the points in form of edges that form a curve.
    Uses either Hermite or Bezier method.
    """
    xcoefs = generate_curve_coefs(x0, x1, x2, x3, curve_type)[0]
    ycoefs = generate_curve_coefs(y0, y1, y2, y3, curve_type)[0]

    i = 1
    while i <= step:
        t = float(i) / step
        x = t * (t * (xcoefs[0] * t + xcoefs[1]) + xcoefs[2]) + xcoefs[3]
        y = t * (t * (ycoefs[0] * t + ycoefs[1]) + ycoefs[2]) + ycoefs[3]

        add_edge(matrix, x0, y0, 0, x, y, 0)
        x0, y0 = x, y
        i += 1


def generate_torus(cx: float, cy: float, cz: float, r0: float, r1: float,
                   step: int) -> list:
    """Generate the points for a torus shape."""
    points = []
    rot_start, rot_stop = 0, step
    circ_start, circ_stop = 0, step

    for rotation in range(rot_start, rot_stop):
        rot = rotation / float(step)
        for circle in range(circ_start, circ_stop):
            circ = circle / float(step)

            points.append([
                cos(2 * pi * rot) * (r0 * cos(2 * pi * circ) + r1) + cx,
                r0 * sin(2 * pi * circ) + cy,
                -1 * sin(2 * pi * rot) * (r0 * cos(2 * pi * circ) + r1) + cz
            ])

    return points


def add_torus(matrix: list, cx: float, cy: float, cz: float, r0: float,
              r1: float, step: int) -> None:
    """Add the points for a torus polygon shape to the given matrix."""
    points = generate_torus(cx, cy, cz, r0, r1, step)

    lat_start, lat_stop = 0, step
    longt_start, longt_stop = 0, step

    for lat in range(lat_start, lat_stop):
        for longt in range(longt_start, longt_stop):
            p0 = lat * step + longt

            if longt == (step - 1):
                p1 = p0 - longt
            else:
                p1 = p0 + 1

            p2, p3 = (p1 + step) % (step * step), (p0 + step) % (step * step)

            add_polygon(matrix, points[p0][0], points[p0][1], points[p0][2],
                        points[p3][0], points[p3][1], points[p3][2],
                        points[p2][0], points[p2][1], points[p2][2])
            add_polygon(matrix, points[p0][0], points[p0][1], points[p0][2],
                        points[p2][0], points[p2][1], points[p2][2],
                        points[p1][0], points[p1][1], points[p1][2])


def add_box(matrix, x, y, z, width, height, depth):
    x1, y1, z1 = x + width, y - height, z - depth

    add_polygon(matrix, x, y, z, x1, y1, z, x1, y, z)
    add_polygon(matrix, x, y, z, x, y1, z, x1, y1, z)

    add_polygon(matrix, x1, y, z1, x, y1, z1, x, y, z1)
    add_polygon(matrix, x1, y, z1, x1, y1, z1, x, y1, z1)

    add_polygon(matrix, x1, y, z, x1, y1, z1, x1, y, z1)
    add_polygon(matrix, x1, y, z, x1, y1, z, x1, y1, z1)

    add_polygon(matrix, x, y, z1, x, y1, z, x, y, z)
    add_polygon(matrix, x, y, z1, x, y1, z1, x, y1, z)

    add_polygon(matrix, x, y, z1, x1, y, z, x1, y, z1)
    add_polygon(matrix, x, y, z1, x, y, z, x1, y, z)

    add_polygon(matrix, x, y1, z, x1, y1, z1, x1, y1, z)
    add_polygon(matrix, x, y1, z, x, y1, z1, x1, y1, z1)


def draw_scanline(x0, z0, x1, z1, y, screen, zbuffer, color):
    if x0 > x1:
        tx, tz = x0, z0
        x0, z0 = x1, z1
        x1, z1 = tx, tz

    x, z = x0, z0

    delta_z = (z1 - z0) / (x1 - x0 + 1) if (x1 - x0 + 1) != 0 else 0

    while x <= x1:
        plot(screen, zbuffer, color, x, y, z)
        x += 1
        z += delta_z


def scanline_convert(matrix: list, i: int, screen: list, zbuffer: list,
                     color: list):
    flip = False
    BOT, TOP, MID = 0, 2, 1

    points = [(matrix[i][0], matrix[i][1], matrix[i][2]),
              (matrix[i + 1][0], matrix[i + 1][1], matrix[i + 1][2]),
              (matrix[i + 2][0], matrix[i + 2][1], matrix[i + 2][2])
              ].sort(key=lambda x: x[1])

    x0, z0, x1, z1, y = points[BOT][0], points[BOT][2], points[BOT][0], points[
        BOT][2], int(points[BOT][1])

    distance0, distance1, distance2 = int(points[TOP][1]) - y * 1.0 + 1, int(
        points[MID][1]) - y * 1.0 + 1, int(
            points[TOP][1]) - int(points[MID][1]) * 1.0 + 1

    dx0 = (points[TOP][0] -
           points[BOT][0]) / distance0 if distance0 != 0 else 0
    dz0 = (points[TOP][2] -
           points[BOT][2]) / distance0 if distance0 != 0 else 0
    dx1 = (points[MID][0] -
           points[BOT][0]) / distance1 if distance1 != 0 else 0
    dz1 = (points[MID][2] -
           points[BOT][2]) / distance1 if distance1 != 0 else 0

    while y <= int(points[TOP][1]):
        if (not flip and y >= int(points[MID][1])):
            flip = True
            dx1 = (points[TOP][0] -
                   points[MID][0]) / distance2 if distance2 != 0 else 0
            dz1 = (points[TOP][2] -
                   points[MID][2]) / distance2 if distance2 != 0 else 0
            x1, z1 = points[MID][0], points[MID][2]

        draw_scanline(int(x0), z0, int(x1), z1, y, screen, zbuffer, color)
        x0 += dx0
        z0 += dz0
        x1 += dx1
        z1 += dz1
        y += 1


def draw_polygons(matrix: list, screen: list, zbuffer: list, view: list,
                  ambient: list, light: list, symbols: dict,
                  reflect: str) -> None:
    if len(matrix) < 2:
        print('Need at least 3 points to draw')
        return

    point = 0

    while point < len(matrix) - 2:
        normal = calculate_normal(matrix, point)

        if normal[2] > 0:
            color = get_lighting(normal, view, ambient, light, symbols,
                                 reflect)
            scanline_convert(matrix, point, screen, zbuffer, color)

        point += 3
