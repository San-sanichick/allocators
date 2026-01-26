#include <exception>
#include <glm/fwd.hpp>
#include <iostream>
#include <optional>
#include <fstream>
#include <string>
#include <cctype>
#include <vector>

#include <glm/vec2.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "vecrender_triangulator.h"
#include <vecrender_path.h>



struct Segment
{
    Segment() = default;
    Segment(const Segment&) = default;
    Segment(Segment&&) = default;

    char command;
    glm::vec2 end;
    std::optional<glm::vec2> c1;
    std::optional<glm::vec2> c2;
};


int main()
{
    // std::ifstream file("path.txt");
    // if (!file.is_open())
    // {
    //     std::cerr << "No file" << std::endl;
    //     return 1;
    // }
    //
    // Segment* temp = nullptr;
    // std::vector<Segment> segments;
    // std::string word;
    //
    // std::vector<double> buffer;
    // buffer.reserve(6);
    //
    // while (file >> word)
    // {
    //     if (word.length() == 1 && std::isalpha(word[0]))
    //     {
    //         if (temp != nullptr)
    //         {
    //             switch (temp->command)
    //             {
    //                 case 'M':
    //                 case 'm':
    //                     temp->end.x = buffer[0];
    //                     temp->end.y = buffer[1];
    //                     break;
    //                 case 'L':
    //                 case 'l':
    //                     temp->end.x = buffer[0];
    //                     temp->end.y = buffer[1];
    //                     break;
    //                 case 'C':
    //                 case 'c':
    //                     temp->end.x = buffer[0];
    //                     temp->end.y = buffer[1];
    //                     temp->c1 = glm::vec2(buffer[2], buffer[3]);
    //                     temp->c2 = glm::vec2(buffer[4], buffer[5]);
    //                     break;
    //                 case 'Q':
    //                 case 'q':
    //                     temp->end.x = buffer[0];
    //                     temp->end.y = buffer[1];
    //                     temp->c1 = glm::vec2(buffer[2], buffer[3]);
    //                     break;
    //                 case 'Z':
    //                     break;
    //             }
    //
    //             segments.push_back(*temp);
    //             buffer.clear();
    //             delete temp;
    //         }
    //
    //         temp = new Segment();
    //         temp->command = word[0];
    //         continue;
    //     }
    //
    //     double num = std::stod(word);
    //     buffer.push_back(num);
    // }
    // file.close();
    //
    // vecrender::Path path;
    //
    // for (const auto& seg : segments)
    // {
    //     switch (seg.command)
    //     {
    //         case 'M':
    //         // case 'm':
    //             path.moveTo(seg.end);
    //             break;
    //         case 'L':
    //         // case 'l':
    //             path.lineTo(seg.end);
    //             break;
    //         case 'C':
    //         // case 'c':
    //             path.cubicTo(seg.end, seg.c1.value(), seg.c2.value());
    //             break;
    //         case 'Q':
    //         // case 'q':
    //             path.quadraticTo(seg.end, seg.c1.value());
    //             break;
    //         case 'Z':
    //             path.close();
    //             break;
    //     }
    // }

    vecrender::Path path;

    path.moveTo({20, 20});
    path.lineTo({100, 20});
    path.lineTo({100, 100});
    path.lineTo({20, 100});
    path.close();

    vecrender::Triangulator triangulator(path);
    const auto& vertices = triangulator.getVertices();
    const auto count = triangulator.getNumVertices();

    std::ofstream out("vertices.json");
    out << '[';
    out << '[';
    for (size_t i = 0; i < count; i++)
    {
        const auto& vertex = vertices[i];

        std::cout
            << glm::to_string(vertex.position)
            << " | "
            << glm::to_string(vertex.factors)
            << std::endl;

        out
            << vertex.position.x << ','
            << vertex.position.y << ','
            << vertex.factors.x << ','
            << vertex.factors.y << ','
            << vertex.factors.z;

        if (i < count - 1) out << ',';
    }
    out << "]," << count << ']' << std::endl;
    out.close();

    return 0;
}
