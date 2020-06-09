from math import (sin, cos)


def new_matrix(rows: int = 4, cols: int = 4) -> list:
    """Create a new matrix."""
    m = []
    for c in range(cols):
        m.append([0 for r in range(rows)])

    return m


def ident(matrix: list) -> None:
    """Change a matrix to an identity matrix."""
    assert len(matrix) == len(matrix[0])

    for r in range(len(matrix[0])):
        for c in range(len(matrix)):
            if r == c:
                matrix[c][r] = 1
            else:
                matrix[c][r] = 0


def make_bezier() -> list:
    """Create a Bezier matrix."""
    return [[-1, 3, -3, 1], [3, -6, 3, 0], [-3, 3, 0, 0], [1, 0, 0, 0]]


def make_hermite() -> list:
    """Create a Hermite matrix."""
    return [[2, -3, 0, 1], [-2, 3, 0, 0], [1, -2, 1, 0], [1, -1, 0, 0]]


def generate_curve_coefs(p0: float, p1: float, p2: float, p3: float,
                         t: str) -> list:
    """Generate coefficients using Bezier or Hermite matrix."""
    coefs = [[p0, p1, p2, p3]]

    if t == "hermite":
        curve = make_hermite()
    else:
        curve = make_bezier()

    matrix_mult(curve, coefs)
    return coefs


def make_translate(x: float, y: float, z: float) -> list:
    """Create a translation matrix."""
    t = new_matrix()
    ident(t)
    t[3][0] = x
    t[3][1] = y
    t[3][2] = z
    return t


def make_scale(x: float, y: float, z: float) -> list:
    """Create a scale matrix."""
    t = new_matrix()
    ident(t)
    t[0][0] = x
    t[1][1] = y
    t[2][2] = z
    return t


def make_rotX(theta: float) -> list:
    """Create a rotation matrix about the x-axis."""
    t = new_matrix()
    ident(t)
    t[1][1] = cos(theta)
    t[2][1] = -1 * sin(theta)
    t[1][2] = sin(theta)
    t[2][2] = cos(theta)
    return t


def make_rotY(theta: float) -> list:
    """Create a rotation matrix about the y-axis."""
    t = new_matrix()
    ident(t)
    t[0][0] = cos(theta)
    t[0][2] = -1 * sin(theta)
    t[2][0] = sin(theta)
    t[2][2] = cos(theta)
    return t


def make_rotZ(theta: float) -> list:
    """Create a rotation matrix about the z-axis."""
    t = new_matrix()
    ident(t)
    t[0][0] = cos(theta)
    t[1][0] = -1 * sin(theta)
    t[0][1] = sin(theta)
    t[1][1] = cos(theta)
    return t


def print_matrix(matrix: list) -> None:
    """Print the given matrix in the transposed format."""
    s = ""
    for r in range(len(matrix[0])):
        for c in range(len(matrix)):
            s += f"{str(matrix[c][r])} "
        s += "\n"
    print(s)


def matrix_mult(m1: list, m2: list) -> None:
    """Multiply m1 by m2 and setting the product to m2."""
    point = 0
    for row in m2:
        tmp = row
        for r in range(4):
            m2[point][r] = (m1[0][r] * tmp[0] + m1[1][r] * tmp[1] +
                            m1[2][r] * tmp[2] + m1[3][r] * tmp[3])
        point += 1
