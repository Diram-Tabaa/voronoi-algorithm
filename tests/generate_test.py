import random 


def get_random_point(xmin, xmax, ymin, ymax, fd):
    x = random.uniform(xmin, xmax)
    y = random.uniform(ymin, ymax)
    print("%.4f %.4f" % (x, y), file = fd)


if __name__ == "__main__":
    xmin, xmax = 0, 10
    ymin, ymax = -10, 0
    fd = open("python_test", "w")
    npts = 30
    print("%d" % npts, file = fd)
    for i in range(npts):
        get_random_point(xmin, xmax, ymin, ymax, fd)
