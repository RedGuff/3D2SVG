#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <utility>

using namespace std;

// Camera parameters
double camera_pos_x = 0.0, camera_pos_y = 0.0, camera_pos_z = 0.0;
double camera_look_x = 0.0, camera_look_y = 0.0, camera_look_z = 0.0;
string camera_projection = "orthogonal";

// Lights
vector<double> light_pos_x;
vector<double> light_pos_y;
vector<double> light_pos_z;
vector<int> light_color_r;
vector<int> light_color_g;
vector<int> light_color_b;

// Spheres
vector<double> sphere_x;
vector<double> sphere_y;
vector<double> sphere_z;
vector<double> sphere_radius;
vector<int> sphere_color_r;
vector<int> sphere_color_g;
vector<int> sphere_color_b;

// Polygons
vector<vector<pair<double, double>>> polygons_vertices; // Each polygon is a vector of 2D points after projection
vector<int> polygon_color_r;
vector<int> polygon_color_g;
vector<int> polygon_color_b;

// Texts
vector<pair<double, double>> text_positions; // 2D positions after projection
vector<string> text_strings;
vector<int> text_color_r;
vector<int> text_color_g;
vector<int> text_color_b;

// Helper function to trim whitespace
string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == string::npos)
        return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

// Helper function to parse a vector from a string like "<x, y, z>"
vector<double> parse_vector(const string& s) {
    vector<double> vec;
    string num;
    stringstream ss(s);
    char ch;
    while (ss >> ch) {
        if (ch == '<') continue;
        if (ch == ',' || ch == '>') {
            if (!num.empty()) {
                vec.push_back(stod(num));
                num.clear();
            }
            continue;
        }
        num += ch;
    }
    return vec;
}

// Function to project a 3D point to 2D
pair<double, double> project_point(double x, double y, double z) {
    double proj_x, proj_y;
    if(camera_projection == "orthogonal") {
        proj_x = x - camera_pos_x;
        proj_y = y - camera_pos_y;
    }
    else { // perspective
        double dz = z - camera_pos_z;
        if(dz == 0) dz = 0.001; // Avoid division by zero
        double factor = 1.0 / dz;
        proj_x = (x - camera_pos_x) * factor;
        proj_y = (y - camera_pos_y) * factor;
    }
    return {proj_x, proj_y};
}

// Function to parse the camera line
void parse_camera(const string& line) {
    // Format: camera <x, y, z>, <look_x, look_y, look_z>, projection
    size_t first_bracket = line.find('<');
    size_t second_bracket = line.find('>', first_bracket);
    string pos_str = line.substr(first_bracket, second_bracket - first_bracket + 1);
    vector<double> pos = parse_vector(pos_str);
    if (pos.size() != 3) {
        cerr << "Invalid camera position.\n";
        return;
    }
    camera_pos_x = pos[0];
    camera_pos_y = pos[1];
    camera_pos_z = pos[2];

    size_t second_comma = line.find(',', second_bracket + 1);
    size_t third_bracket = line.find('<', second_comma);
    size_t fourth_bracket = line.find('>', third_bracket);
    string look_str = line.substr(third_bracket, fourth_bracket - third_bracket + 1);
    vector<double> look = parse_vector(look_str);
    if (look.size() != 3) {
        cerr << "Invalid camera look_at.\n";
        return;
    }
    camera_look_x = look[0];
    camera_look_y = look[1];
    camera_look_z = look[2];

    size_t last_comma = line.find_last_of(',');
    string projection = trim(line.substr(last_comma + 1));
    camera_projection = projection;
}

// Function to parse the light line
void parse_light(const string& line) {
    // Format: light <x, y, z>, <r, g, b>
    size_t first_bracket = line.find('<');
    size_t second_bracket = line.find('>', first_bracket);
    string pos_str = line.substr(first_bracket, second_bracket - first_bracket + 1);
    vector<double> pos = parse_vector(pos_str);
    if (pos.size() != 3) {
        cerr << "Invalid light position.\n";
        return;
    }
    light_pos_x.push_back(pos[0]);
    light_pos_y.push_back(pos[1]);
    light_pos_z.push_back(pos[2]);

    size_t second_comma = line.find(',', second_bracket + 1);
    size_t third_bracket = line.find('<', second_comma);
    size_t fourth_bracket = line.find('>', third_bracket);
    string color_str = line.substr(third_bracket, fourth_bracket - third_bracket + 1);
    vector<double> color = parse_vector(color_str);
    if (color.size() != 3) {
        cerr << "Invalid light color.\n";
        return;
    }
    light_color_r.push_back(static_cast<int>(color[0]));
    light_color_g.push_back(static_cast<int>(color[1]));
    light_color_b.push_back(static_cast<int>(color[2]));
}

// Function to parse the sphere line
void parse_sphere(const string& line) {
    // Format: sphere <x, y, z>, radius, <r, g, b>
    size_t first_bracket = line.find('<');
    size_t second_bracket = line.find('>', first_bracket);
    string pos_str = line.substr(first_bracket, second_bracket - first_bracket + 1);
    vector<double> pos = parse_vector(pos_str);
    if (pos.size() != 3) {
        cerr << "Invalid sphere position.\n";
        return;
    }
    sphere_x.push_back(pos[0]);
    sphere_y.push_back(pos[1]);
    sphere_z.push_back(pos[2]);

    size_t comma_after_pos = line.find(',', second_bracket + 1);
    size_t next_comma = line.find(',', comma_after_pos + 1);
    string radius_str = trim(line.substr(comma_after_pos + 1, next_comma - comma_after_pos - 1));
    double radius = stod(radius_str);
    sphere_radius.push_back(radius);

    size_t third_bracket = line.find('<', next_comma);
    size_t fourth_bracket = line.find('>', third_bracket);
    string color_str = line.substr(third_bracket, fourth_bracket - third_bracket + 1);
    vector<double> color = parse_vector(color_str);
    if (color.size() != 3) {
        cerr << "Invalid sphere color.\n";
        return;
    }
    sphere_color_r.push_back(static_cast<int>(color[0]));
    sphere_color_g.push_back(static_cast<int>(color[1]));
    sphere_color_b.push_back(static_cast<int>(color[2]));
}

// Function to parse the polygon line
void parse_polygon(const string& line) {
    // Format: polygon <x1, y1, z1>, <x2, y2, z2>, ..., <r, g, b>
    size_t first_bracket = line.find('<');
    size_t last_bracket = line.rfind('>');
    if (first_bracket == string::npos || last_bracket == string::npos || last_bracket <= first_bracket) {
        cerr << "Invalid polygon format.\n";
        return;
    }
    // Extract all vertex strings
    vector<string> vertex_strs;
    size_t pos = first_bracket;
    while (pos < last_bracket) {
        size_t start = line.find('<', pos);
        if (start == string::npos || start >= last_bracket) break;
        size_t end = line.find('>', start);
        if (end == string::npos || end > last_bracket) break;
        vertex_strs.push_back(line.substr(start, end - start + 1));
        pos = end + 1;
    }
    // The last vertex is actually the color
    if (vertex_strs.size() < 2) {
        cerr << "Polygon must have at least one vertex and color.\n";
        return;
    }
    // The last vertex string is the color
    string color_str = vertex_strs.back();
    vertex_strs.pop_back();
    vector<double> color = parse_vector(color_str);
    if (color.size() != 3) {
        cerr << "Invalid polygon color.\n";
        return;
    }
    polygon_color_r.push_back(static_cast<int>(color[0]));
    polygon_color_g.push_back(static_cast<int>(color[1]));
    polygon_color_b.push_back(static_cast<int>(color[2]));

    // Parse vertices
    vector<pair<double, double>> projected_vertices; // Temporarily store projected 2D vertices
    for(const string& v_str : vertex_strs){
        vector<double> vertex = parse_vector(v_str);
        if (vertex.size() != 3) {
            cerr << "Invalid polygon vertex.\n";
            return;
        }
        // Project the vertex
        pair<double, double> proj = project_point(vertex[0], vertex[1], vertex[2]);
        projected_vertices.push_back(proj);
    }
    polygons_vertices.push_back(projected_vertices);
}

// Function to parse the text line
void parse_text(const string& line) {
    // Format: text <x, y, z>, "string", <r, g, b>
    size_t first_bracket = line.find('<');
    size_t second_bracket = line.find('>', first_bracket);
    string pos_str = line.substr(first_bracket, second_bracket - first_bracket + 1);
    vector<double> pos = parse_vector(pos_str);
    if (pos.size() != 3) {
        cerr << "Invalid text position.\n";
        return;
    }

    // Find the string enclosed in quotes
    size_t first_quote = line.find('"', second_bracket);
    size_t second_quote = line.find('"', first_quote + 1);
    if (first_quote == string::npos || second_quote == string::npos) {
        cerr << "Invalid text string.\n";
        return;
    }
    string text_str = line.substr(first_quote + 1, second_quote - first_quote - 1);

    // Find the color
    size_t color_bracket = line.find('<', second_quote);
    size_t color_end = line.find('>', color_bracket);
    string color_str = line.substr(color_bracket, color_end - color_bracket + 1);
    vector<double> color = parse_vector(color_str);
    if (color.size() != 3) {
        cerr << "Invalid text color.\n";
        return;
    }

    // Project the position
    pair<double, double> proj = project_point(pos[0], pos[1], pos[2]);

    text_positions.push_back(proj);
    text_strings.push_back(text_str);
    text_color_r.push_back(static_cast<int>(color[0]));
    text_color_g.push_back(static_cast<int>(color[1]));
    text_color_b.push_back(static_cast<int>(color[2]));
}

// Function to write the SVG header
void write_svg_header(ofstream& svg, int width, int height) {
    svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" ";
    svg << "width=\"" << width << "\" height=\"" << height << "\">\n";
    // Optionally, add a background
    svg << "<rect width=\"100%\" height=\"100%\" fill=\"black\" />\n";
}

// Function to write the SVG footer
void write_svg_footer(ofstream& svg) {
    svg << "</svg>";
}

// Function to write spheres as circles
void write_spheres_svg(ofstream& svg, int width, int height) {
    for(int i = 0; i < sphere_x.size(); i++) {
        pair<double, double> proj = project_point(sphere_x[i], sphere_y[i], sphere_z[i]);
        // Transform to SVG coordinates (assuming center is at width/2, height/2)
        double svg_x = width / 2 + proj.first * 100; // Scale factor
        double svg_y = height / 2 - proj.second * 100; // Invert y-axis for SVG
        double radius = sphere_radius[i] * 100; // Scale radius
        svg << "<circle cx=\"" << svg_x << "\" cy=\"" << svg_y
             << "\" r=\"" << radius
             << "\" fill=\"rgb(" << sphere_color_r[i] << ","
             << sphere_color_g[i] << "," << sphere_color_b[i] << ")\" />\n";
    }
}

// Function to write polygons
void write_polygons_svg(ofstream& svg, int width, int height) {
    for(int i = 0; i < polygons_vertices.size(); i++) {
        svg << "<polygon points=\"";
        for(auto& point : polygons_vertices[i]) {
            double svg_x = width / 2 + point.first * 100; // Scale factor
            double svg_y = height / 2 - point.second * 100; // Invert y-axis for SVG
            svg << svg_x << "," << svg_y << " ";
        }
        svg << "\" fill=\"rgb(" << polygon_color_r[i] << ","
             << polygon_color_g[i] << "," << polygon_color_b[i] << ")\" />\n";
    }
}

// Function to write texts
void write_texts_svg(ofstream& svg, int width, int height) {
    for(int i = 0; i < text_positions.size(); i++) {
        double svg_x = width / 2 + text_positions[i].first * 100; // Scale factor
        double svg_y = height / 2 - text_positions[i].second * 100; // Invert y-axis for SVG
        svg << "<text x=\"" << svg_x << "\" y=\"" << svg_y
             << "\" fill=\"rgb(" << text_color_r[i] << ","
             << text_color_g[i] << "," << text_color_b[i] << ")\" ";
        svg << "font-size=\"12\" text-anchor=\"middle\">" << text_strings[i] << "</text>\n";
    }
}

int main() {
    string scene_filename;
    string svg_filename;

    // Get input and output filenames
    cout << "Enter the scene file name: ";
    cin >> scene_filename;

    ifstream scene_file(scene_filename);
    if(!scene_file.is_open()) {
        cerr << "Failed to open scene file.\n";
        return 1;
    }
    cout << "Enter the output SVG file name: ";
    cin >> svg_filename;



    string line;
    while(getline(scene_file, line)) {
        line = trim(line);
        if(line.empty() || line[0] == '#') continue; // Skip empty lines and comments
        if(line.find("camera") == 0) {
            parse_camera(line);
        }
        else if(line.find("light") == 0) {
            parse_light(line);
        }
        else if(line.find("sphere") == 0) {
            parse_sphere(line);
        }
        else if(line.find("polygon") == 0) {
            parse_polygon(line);
        }
        else if(line.find("text") == 0) {
            parse_text(line);
        }
        else {
            cerr << "Unknown line type: " << line << "\n";
        }
    }
    scene_file.close();

    // Define SVG canvas size
    int svg_width = 800;
    int svg_height = 600;

    ofstream svg_file(svg_filename);
    if(!svg_file.is_open()) {
        cerr << "Failed to open SVG file for writing.\n";
        return 1;
    }

    write_svg_header(svg_file, svg_width, svg_height);
    write_spheres_svg(svg_file, svg_width, svg_height);
    write_polygons_svg(svg_file, svg_width, svg_height);
    write_texts_svg(svg_file, svg_width, svg_height);
    write_svg_footer(svg_file);

    svg_file.close();

    cout << "SVG file generated successfully.\n";

    return 0;
}
