#include <iostream>
#include <bitset>
#include <vector>
#include <cstdint>
#include <algorithm>

std::bitset<400> west_mask, east_mask;

void initialize_masks() {
    std::bitset<400> tmp;
    for (int i = 0; i < 20; ++i)
        tmp.set(20*i);
    west_mask = ~tmp;

    tmp.reset();
    for (int i = 0; i < 20; ++i) 
        tmp.set(20*i+19);
    east_mask = ~tmp;
}

class BitBoard {
private:
    std::bitset<400> bits;

    std::bitset<400> outline_flood() const {
        std::bitset<400> flood = bits;
        flood |= (bits >> 1) & east_mask;
        flood |= (bits << 1) & west_mask;
        flood |= (bits >> 20);
        flood |= (bits << 20);
        return flood;
    }
public:
    BitBoard() : bits() {}
    BitBoard(std::bitset<400> bits) : bits(bits) {}

    BitBoard outline() const {
        return outline_flood();
    }

    BitBoard corners() const {
        std::bitset<400> flood;
        flood |= ((bits << 19) | (bits >> 21)) & east_mask;
        flood |= ((bits >> 19) | (bits << 21)) & west_mask;
        return BitBoard(flood & ~outline_flood());
    }

    void debug_print() const {
        for (int i = 0; i < 400; ++i) {
            if (i % 20 == 0) {
                std::cout << std::endl;
            }

            std::cout << bits[i];
        }

        std::cout << std::endl;
    }

    BitBoard operator|(const BitBoard& other) const {
        return BitBoard(bits | other.bits);
    }

    BitBoard operator~() const {
        return BitBoard(~bits);
    }
};

class Board {
private:
    BitBoard blue, yellow, red, green;
public:
    Board() : blue(), yellow(), red(), green() {}
};

struct Point {
    int x;
    int y;

    Point(int x, int y) : x(x), y(y) {}

    bool operator<(const Point& other) const {
        if (y == other.y) {
            return x < other.x;
        } else {
            return y < other.y;
        }
    }

    bool operator==(const Point& other) const {
        return (x == other.x) && (y == other.y);
    }
};

class Shape {
private:
    std::vector<Point> points;
    std::vector<Point> attach;
    int width; 
    int height;
public:
    Shape(std::vector<Point> tiles) : points(tiles) {
        std::sort(points.begin(), points.end());
        height = points.back().y;

        int mailbox[25] = {0};
        int max_x = -1;
        for (const Point& p : points) {
            mailbox[p.y*5 + p.x] = 1;
            if (max_x < p.x) {
                max_x = p.x;
            }
        }

        width = max_x;
        
        for (const Point& p : points) {
            int i = p.y*5 + p.x;

            bool empty_left = (p.x == 0) || (mailbox[i-1] == 0);
            bool empty_right = (p.x == 5) || (mailbox[i+1] == 0);
            if (!(empty_left || empty_right))
                continue;
            
            bool empty_up = (p.y == 0) || (mailbox[i-5] == 0);
            bool empty_down = (p.y == 5) || (mailbox[i+5] == 0);
            if (!(empty_up || empty_down))
                continue;

            attach.push_back(p);
        }
    }
    
    Shape flip_vertical() const {
        Shape flipped = *this;

        for (Point& p : flipped.points) {
            p.y = height - p.y;
        }

        for (Point& p : flipped.attach) {
            p.y = height - p.y;
        }

        std::sort(flipped.points.begin(), flipped.points.end());
        std::sort(flipped.attach.begin(), flipped.attach.end());
        return flipped;
    } 

    Shape flip_horizontal() const {
        Shape flipped = *this;

        for (Point& p : flipped.points) {
            p.x = width - p.x;
        }

        for (Point& p : flipped.attach) {
            p.x = width - p.x;
        }

        std::sort(flipped.points.begin(), flipped.points.end());
        std::sort(flipped.attach.begin(), flipped.attach.end());
        return flipped;
    }

    Shape turn() const {
        Shape turned = *this;

        for (Point& p : turned.points) {
            std::swap(p.x, p.y);
        }

        for (Point& p : turned.attach) {
            std::swap(p.x, p.y);
        }

        std::swap(turned.width, turned.height);

        std::sort(turned.points.begin(), turned.points.end());
        std::sort(turned.attach.begin(), turned.attach.end());
        return turned;
    }

    std::vector<Shape> orientations() const { 
        std::vector<Shape> shapes;
        Shape horizontal = flip_horizontal();
        Shape vertical = flip_vertical();
        Shape turned = turn();
        shapes.push_back(*this);
        shapes.push_back(horizontal);
        shapes.push_back(vertical);
        shapes.push_back(vertical.flip_horizontal());
        shapes.push_back(turned);
        shapes.push_back(turned.flip_horizontal());
        Shape tmp = turned.flip_vertical();
        shapes.push_back(tmp);
        shapes.push_back(tmp.flip_horizontal());

        std::sort(shapes.begin(), shapes.end());
        shapes.erase(std::unique(shapes.begin(), shapes.end()), shapes.end());
        return shapes;
    }

    bool equal(const Shape& other) const {
        return (width == other.width) && (height == other.height) && (points == other.points) && (attach == other.attach);
    }

    bool operator==(const Shape& other) const {
        return equal(other);
    }

    void debug_print() const {
        for (int y = 0; y <= height; ++y) {
            for (int x = 0; x <= width; ++x) {
                Point p(x, y);
                if (std::find(attach.begin(), attach.end(), p) != attach.end()) {
                    std::cout << "A";
                } else if (std::find(points.begin(), points.end(), p) != points.end()) {
                    std::cout << "P";
                } else {
                    std::cout << " ";
                }
            }

            std::cout << std::endl;
        }

        for (Point p : attach) {
            std::cout << "(" << p.x << ", " << p.y << ")" << std::endl;
        }
    }

    bool operator<(const Shape& other) const {
        return points.back() < other.points.back();
    }
};

Shape piece_shapes[21] = {
    Shape({{0,0}}),
    Shape({{0,0}, {1,0}}),
    Shape({{0,0}, {1,0}, {1,1}}),
    Shape({{0,0}, {1,0}, {2,0}}),
    Shape({{0,0}, {1,0}, {0,1}, {1,1}}),
    Shape({{1,0}, {0,1}, {1,1}, {2,1}}),
    Shape({{0,0}, {1,0}, {2,0}, {3,0}}),
    Shape({{2,0}, {0,1}, {1,1}, {2,1}}),
    Shape({{1,0}, {2,0}, {0,1}, {1,1}}),
    Shape({{0,0}, {0,1}, {1,1}, {2,1}, {3,1}}),
    Shape({{1,0}, {1,1}, {0,2}, {1,2}, {2,2}}),
    Shape({{0,0}, {0,1}, {0,2}, {1,2}, {2,2}}),
    Shape({{1,0}, {2,0}, {3,0}, {0,1}, {1,1}}),
    Shape({{2,0}, {0,1}, {1,1}, {2,1}, {0,2}}),
    Shape({{0,0}, {1,0}, {2,0}, {3,0}, {4,0}}),
    Shape({{0,0}, {0,1}, {1,1}, {0,2}, {1,2}}),
    Shape({{1,0}, {2,0}, {0,1}, {1,1}, {0,2}}),
    Shape({{0,0}, {1,0}, {0,1}, {0,2}, {1,2}}),
    Shape({{1,0}, {2,0}, {0,1}, {1,1}, {1,2}}),
    Shape({{1,0}, {0,1}, {1,1}, {2,1}, {1,2}}),
    Shape({{1,0}, {0,1}, {1,1}, {2,1}, {3,1}}),
};

int main() {
    initialize_masks();

    for (int i = 0; i < 21; ++i) {
        piece_shapes[i].debug_print();
        std::cout << std::endl;
    }
    
    return 0;
}

