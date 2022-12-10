//
// Created by nektt on 27/11/2022.
//

#include "Figures.h"
#include <algorithm>


/////////////////////////////////////////// GENERAL ///////////////////////////////////////////////

int fig::getNumberFromRegex(const std::regex &reg, const std::string &svgStr) {
    std::smatch match;
    static const std::regex numberR(R"("\d+)");
    std::regex_search(std::cbegin(svgStr), std::cend(svgStr), match, reg);
    if (match.length() == 0) throw std::runtime_error("svg format err");
    std::regex_search(std::cbegin(match[0].str()), std::cend(match[0].str()), match, numberR);
    std::string ss = match[0].str();
    std::string s(std::begin(ss) + 1, std::end(ss));
    return std::stoi(s);
}


std::ostream &operator<<(std::ostream &out, const fig::Point &point) {
    out << "(" << point.x << " : " << point.y << ")";
    return out;
}


bool operator==(const fig::figVariants &left, const fig::figVariants &right) {
    if (left.index() != right.index()) return false;
    switch (left.index()) {
        case fig::CIRCLE:
            return std::get<fig::Circle>(left) == std::get<fig::Circle>(right);
        case fig::LINE:
            return std::get<fig::Line>(left) == std::get<fig::Line>(right);
        case fig::RECT:
            return std::get<fig::Rect>(left) == std::get<fig::Rect>(right);
        case fig::POLYGON:
            return std::get<fig::Polygon>(left) == std::get<fig::Polygon>(right);
        case fig::PATH:
            return std::get<fig::Path>(left) == std::get<fig::Path>(right);
    }
    return false;
}


double fig::getDistance(fig::Point a, fig::Point b) {
    return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}


/////////////////////////////////////////// POINT /////////////////////////////////////////////////

bool fig::Point::operator==(const fig::Point &other) const {
    return x == other.x && y == other.y;
}

bool fig::Point::operator!=(const fig::Point &other) const {
    return x != other.x || y != other.y;
}


/////////////////////////////////////////// CIRCLE ////////////////////////////////////////////////

void fig::Circle::print() const {
    std::cout << "Circle: circle = " << getCenter() << ", r = " << getRadius();
}

void fig::Circle::parseSvg(const std::string &svgStr) {
// Format example:
// <circle cx="150" cy="100" r="80" fill="green" />
//
    std::regex cxR(R"(cx\s*=\s*"\d+")"),
            cyR(R"(cy\s*=\s*"\d+")"), rR(R"(r\s*=\s*"\d+")");

    m_c.x = getNumberFromRegex(cxR, svgStr);
    m_c.y = getNumberFromRegex(cyR, svgStr);
    m_r = getNumberFromRegex(rR, svgStr);
}


// friend

bool fig::operator==(const fig::Circle &left, const fig::Circle &right) {
    return (left.m_c == right.m_c) && (left.m_r == right.m_r);
}

bool fig::doesOverlap(const fig::Circle &left, fig::figVariants rightVariant) {
    switch (rightVariant.index()) {
        case fig::CIRCLE: {
            auto *right = std::get_if<fig::Circle>(&rightVariant);
            double d = fig::getDistance(left.getCenter(), right->getCenter());
            return (left.getRadius() >= right->getRadius()) &&
                   (d + right->getRadius() <= left.getRadius());
        }
        case fig::LINE: {
            auto *right = std::get_if<fig::Line>(&rightVariant);
            bool firstInCircle = left.pointBelongs(right->getPoint1());
            bool secondInCircle = left.pointBelongs(right->getPoint2());

            return firstInCircle && secondInCircle;
        }
        case fig::RECT: {
            auto *right = std::get_if<fig::Rect>(&rightVariant);
            fig::Point p1 = right->getPosition();
            fig::Point p2{p1.x, p1.y + right->getHeight()};
            fig::Point p3{p1.x + right->getWidth(), p1.y};
            fig::Point p4{p1.x + right->getWidth(), p1.y + right->getHeight()};
            return (left.pointBelongs(p1) && left.pointBelongs(p2) && left.pointBelongs(p3) &&
                    left.pointBelongs(p4));
        }
        case fig::PATH: {
            auto *right = std::get_if<fig::Path>(&rightVariant);
            return (std::ranges::all_of((*right).begin(), (*right).end(),
                                        [left](fig::Point p) { return left.pointBelongs(p); }));
        }
        case fig::POLYGON: {
            auto *right = std::get_if<fig::Polygon>(&rightVariant);
            return (std::ranges::all_of((*right).begin(), (*right).end(),
                                        [left](fig::Point p) { return left.pointBelongs(p); }));
        }
    }

}


/////////////////////////////////////////// LINE //////////////////////////////////////////////////

void fig::Line::parseSvg(const std::string &svgStr) {
// Format example:
// <line x1="0" y1="80" x2="100" y2="20" stroke="black" />
//
    std::regex numberR(R"(\d+)"), x1R(R"(x1\s*=\s*"\d+")"),
            x2R(R"(x2\s*=\s*"\d+")"), y1R(R"(y1\s*=\s*"\d+")"), y2R(R"(y2\s*=\s*"\d+")");

    one.x = getNumberFromRegex(x1R, svgStr);
    one.y = getNumberFromRegex(y1R, svgStr);
    two.x = getNumberFromRegex(x2R, svgStr);
    two.y = getNumberFromRegex(y2R, svgStr);
}

void fig::Line::print() const {
    std::cout << "Line: p1 = " << getPoint1() << ", p2 = " << getPoint2();
}


// friend

bool fig::doesOverlap(const fig::Line &left, figVariants rightVariant) {
    return false; //TODO

}


bool fig::operator==(const fig::Line &left, const fig::Line &right) {
    return (left.one == right.one) && (left.two == right.two);
}



/////////////////////////////////////////// RECT //////////////////////////////////////////////////

void fig::Rect::parseSvg(const std::string &svgStr) {
// Format example:
// <line x1="0" y1="80" x2="100" y2="20" stroke="black" />
//
    std::regex xR(R"(x\s*=\s*"\d+")"), yR(R"(y\s*=\s*"\d+")"),
            widthR(R"(width\s*=\s*"\d+?")"), heightR(R"(height\s*=\s*"\d+?")");
    std::smatch match;

    m_width = getNumberFromRegex(widthR, svgStr);
    m_height = getNumberFromRegex(heightR, svgStr);

    if (std::regex_search(svgStr, match, xR))
        m_coord.x = getNumberFromRegex(xR, svgStr);
    if (std::regex_search(svgStr, match, yR))
        m_coord.y = getNumberFromRegex(yR, svgStr);

}

void fig::Rect::print() const {
    std::cout << "Rect: pos = " << getPosition() << ", width = " << getWidth() << ", height = " <<
              getHeight();
}


// friend

bool fig::operator==(const fig::Rect &left, const fig::Rect &right) {
    return (left.m_width == right.m_width) && (left.m_height == right.m_height)
           && (left.m_coord == right.m_coord);
}

bool fig::doesOverlap(const fig::Rect &left, fig::figVariants rightVariant) {
    switch (rightVariant.index()) {
        case fig::CIRCLE: {
            auto *right = std::get_if<fig::Circle>(&rightVariant);
            return (left.pointBelongs({right->getCenter().x, right->getCenter().y + right->getRadius()})
                    &&
                    left.pointBelongs({right->getCenter().x, right->getCenter().y - right->getRadius()})
                    &&
                    left.pointBelongs({right->getCenter().x + right->getRadius(), right->getCenter().y})
                    &&
                    left.pointBelongs({right->getCenter().x - right->getRadius(), right->getCenter().y}));
        }
        case fig::RECT: {
            auto *right = std::get_if<fig::Rect>(&rightVariant);
            fig::Point rightCoord = right->getPosition();
            int rHeight = right->getHeight(), rWidth = right->getWidth();
            return (left.pointBelongs(rightCoord) &&
                    left.pointBelongs({rightCoord.x + rWidth, rightCoord.y}) &&
                    left.pointBelongs({rightCoord.x + rWidth, rightCoord.y + rHeight}) &&
                    left.pointBelongs({rightCoord.x, rightCoord.y + rHeight}));
        }
        case fig::LINE: {
            auto *right = std::get_if<fig::Line>(&rightVariant);
            return (left.pointBelongs(right->getPoint1()) && left.pointBelongs(right->getPoint2()));
        }
        case fig::PATH: {
            auto *right = std::get_if<fig::Path>(&rightVariant);
            return (std::ranges::all_of((*right).begin(), (*right).end(),
                                        [left](fig::Point p) { return left.pointBelongs(p); }));
        }
        case fig::POLYGON: {
            auto *right = std::get_if<fig::Polygon>(&rightVariant);
            return (std::ranges::all_of((*right).begin(), (*right).end(),
                                        [left](fig::Point p) { return left.pointBelongs(p); }));

        }

    }


    return false; //TODO implement
}


/////////////////////////////////////////// POLYGON //////////////////////////////////////////////////

void fig::Polygon::parseSvg(const std::string &svgStr) {
    std::regex pointsR(R"(\d+,\d+)");
    std::smatch match;

    auto words_begin =
            std::sregex_iterator(svgStr.begin(), svgStr.end(), pointsR);
    auto words_end = std::sregex_iterator();
    if (words_begin == words_end) throw std::runtime_error("svg format error");

    int x, y, sepPos;
    std::string match_str;
    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        match = *i;
        match_str = match.str();
        sepPos = match_str.find(',');
        x = std::stoi(std::string(match_str.cbegin(), match_str.cbegin() + sepPos));
        y = std::stoi(std::string(match_str.cbegin() + sepPos + 1, match_str.cend()));
        m_points.push_back(fig::Point{x, y});
    }
}

void fig::Polygon::print() const {
    std::cout << "Polygon: points = ";
    for (auto p: m_points)
        std::cout << p << ", ";
}


// friend

bool fig::operator!=(const fig::Polygon &left, const fig::Polygon &right) {
    return !(left == right);
}

bool fig::operator==(const fig::Polygon &left, const fig::Polygon &right) {
    if (left.m_points.size() != right.m_points.size()) return false;
    if (left.m_points.size() == 0) return true;

    auto leftIter = left.m_points.cbegin();
    auto rightIter = right.m_points.cbegin();
    for (; rightIter != right.m_points.cend(); ++rightIter)
        if (*rightIter == *leftIter) break;

    if (rightIter == right.m_points.cend())
        return false;

    for (; leftIter + 1 != left.m_points.cend();) {
        ++leftIter;
        if (rightIter + 1 == right.m_points.cend())
            rightIter = right.m_points.cbegin();
        else ++rightIter;

        if (*rightIter != *leftIter) return false;
    }
    return true;
}

bool fig::doesOverlap(const fig::Polygon &left, const fig::figVariants &right) {
    return false; //TODO implement
}

/////////////////////////////////////////// PATH //////////////////////////////////////////////////

void fig::Path::parseSvg(const std::string &svgStr) {
    std::regex pointsR(R"(\d+,\d+)");
    std::smatch match;

    auto words_begin =
            std::sregex_iterator(svgStr.begin(), svgStr.end(), pointsR);
    auto words_end = std::sregex_iterator();
    if (words_begin == words_end) throw std::runtime_error("svg format error");

    int x, y, sepPos;
    std::string match_str;
    Point firstP;
    bool isFirst = true;
    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        match = *i;
        match_str = match.str();
        sepPos = match_str.find(',');
        x = std::stoi(std::string(match_str.cbegin(), match_str.cbegin() + sepPos));
        y = std::stoi(std::string(match_str.cbegin() + sepPos + 1, match_str.cend()));
        m_points.push_back(fig::Point{x, y});
        if (isFirst) {
            firstP = fig::Point{x, y};
            isFirst = false;
        }
    }
    if (m_fromPolygon)
        m_points.push_back(firstP);
}

void fig::Path::print() const {
    std::cout << "Path: points = ";
    for (auto p: m_points)
        std::cout << p << ", ";
}


// friend

bool fig::operator==(const fig::Path &left, const fig::Path &right) {
    if (left.m_points.size() != right.m_points.size()) return false;
    for (auto l = left.m_points.cbegin(), r = right.m_points.cbegin();
         l != left.m_points.cend() && r != right.m_points.cend(); ++r, ++l) {
        if (!(*l == *r)) return false;
    }
    return true;
}

bool fig::doesOverlap(const fig::Path &left, const fig::figVariants &right) {
    return false; //TODO implement
}