#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include "Script.hpp"
#include "PNG.hpp"
#include "XPM2.hpp"


using namespace std;

namespace prog {
    // Use to read color values from a script file.
    istream& operator>>(istream& input, Color& c) {
        int r, g, b;
        input >> r >> g >> b;
        c.red() = r;
        c.green() = g;
        c.blue() = b;
        return input;
    }

    Script::Script(const string& filename) :
            image(nullptr), input(filename) {

    }
    void Script::clear_image_if_any() {
        if (image != nullptr) {
            delete image;
            image = nullptr;
        }
    }

    Script::~Script() {
        clear_image_if_any();
    }

    void Script::run() {
        string command;
        while (input >> command) {
            cout << "Executing command '" << command << "' ..." << endl;
            if (command == "open") {
                open();
                continue;
            }
            if (command == "blank") {
                blank();
                continue;
            }
            // Other commands require an image to be previously loaded.
            if (command == "save") {
                save();
                continue;
            } 
            if (command == "invert") {
                invert();
                continue;
            }
            if (command == "to_gray_scale"){
                to_gray_scale();
                continue;
            }
            if (command == "replace"){
                int r1, g1, b1, r2, g2, b2;
                input >> r1 >> g1 >> b1 >> r2 >> g2 >> b2; 
                replace(r1, g1, b1, r2, g2, b2);
                continue;
            }
            // TODO ...
            if (command == "fill") {
                int x, y, w, h, r, g, b;
                input >> x >> y >> w >> h >> r >> g >> b;
                fill(x, y, w, h, r, g, b);
                continue;
            }

            if (command == "h_mirror") {
                h_mirror();
                continue;
            }

            if (command == "v_mirror") {
                v_mirror();
                continue;
            }

            if (command == "add") {
                std::string filename;
                int r, g, b, x, y;
                input >> filename >> r >> g >> b >> x >> y;
                add(filename, r, g, b, x, y);
                continue;
            }

            if (command == "crop"){
                int x,y,w,h;
                input >> x >> y >> w >> h;
                crop(x, y, w, h);
                continue;
            }

            if (command == "rotate_left") {
                rotate_left();
                continue;
            }

            if (command == "rotate_right") {
                rotate_right();
                continue;
            }

            if (command == "median_filter") {
                int ws;
                input >> ws;
                median_filter(ws);
                continue;
            }
        }
    }
    
    bool operator==(prog::Color c1, prog::Color c2){
        //Overload do == para simplificar as funções
        if (c1.red() == c2.red() && c1.green() == c2.green() && c1.blue() == c2.blue()) return true;
        return false;
    }


    void Script::open() {
        // Replace current image (if any) with image read from PNG file.
        clear_image_if_any();
        string filename;
        input >> filename;
        image = loadFromPNG(filename);
    }

    void Script::blank() {
        // Replace current image (if any) with blank image.
        clear_image_if_any();
        int w, h;
        Color fill;
        input >> w >> h >> fill;
        image = new Image(w, h, fill);
    }

    void Script::save() {
        // Save current image to PNG file.
        string filename;
        input >> filename;
        saveToPNG(filename, image);
    }

    void Script::invert() {
        // Inverts the color of every pixel
        for (int h = 0; h < image->height(); h++){
            for (int w = 0; w < image->width();w++){
                Color& pixel = image->at(w,h);
                pixel.red() = 255 - pixel.red();
                pixel.green() = 255 - pixel.green();
                pixel.blue() = 255 - pixel.blue();
            }
        }
    }

    void Script::to_gray_scale() {
        // Transforms every pixel (r,g,b) into (v,v,v) where v = (r + g + b)/3  (int division)
        for (int h = 0; h < image->height(); h++){
            for (int w = 0; w < image->width();w++){
                Color& pixel = image->at(w,h);
                int v = (pixel.red() + pixel.blue() + pixel.green())/3;
                pixel.red() = v;
                pixel.green() = v;
                pixel.blue() = v;
            }
        }
    }
    
    void Script::replace(int r1, int g1, int b1, int r2, int g2, int b2){
        // replaces every (r1,g1,b1) pixel into (r2, g2, b2) 
        for (int h = 0; h < image->height(); h++){
            for (int w = 0; w < image->width();w++){
                Color& pixel = image->at(w,h);
                if (pixel.red() == r1 && pixel.green() == g1 && pixel.blue() == b1){
                    pixel.red() = r2;
                    pixel.green() = g2;
                    pixel.blue() = b2;
                }        
            }
        }
    }

    void Script::fill(int x, int y, int w, int h, int r, int g, int b) {
        // Fills a square (from point (x,y) width w and height h) with a solid (r, g, b) colour
        for (int i = x; i < x+w; i++){
            for (int j = y; j < y+h; j++){
                Color& pixel = image->at(i,j);
                pixel.red() = r;
                pixel.green() = g;
                pixel.blue() = b;
            }
        }
    }

    void Script::h_mirror() {
        // Mirrors the image horizontally
        for (int y = 0; y < image->height(); y++) {
            for (int x = 0; x < image->width() / 2; x++) {
                Color& left = image->at(x, y);
                Color& right = image->at(image->width() - 1 - x, y);
                std::swap(left, right);
            }
        }
    }

    void Script::v_mirror() {
        // Mirrors the image vertically
        for (int y = 0; y < image->height() / 2; y++) {
            for (int x = 0; x < image->width(); x++) {
                Color temp = image->at(x, y);
                image->at(x, y) = image->at(x, image->height() - 1 - y);
                image->at(x, image->height() - 1 - y) = temp;
            }
        }
    }

    void Script::add(const std::string& filename, int r, int g, int b, int x, int y) {
        // Copy all pixel from filename.png that aren't of neutral color (r,g,b) to a rectangle in the current image starting from the point (x,y) 
        Image* newimage = loadFromPNG(filename);
        for (int h = 0; h < newimage->height(); h++) {
            for (int w = 0; w < newimage->width(); w++) {
                if(newimage->at(w,h) == Color(r,g,b)) continue;
                else{
                    image->at(w+x,h+y).red() = newimage->at(w,h).red();
                    image->at(w+x,h+y).green() = newimage->at(w,h).green();
                    image->at(w+x,h+y).blue() = newimage->at(w,h).blue();
                }
            }
        }
        delete newimage;
    }

    void Script::crop(int x,int y,int w,int h){
        Image* newimage = new Image(w, h);
        for (int a = x; a < w+x; a++){
            for (int b = y; b < h+y; b++){
                newimage->at(a-x, b-y) = image->at(a, b);
            }
        }
        delete image;
        image = newimage;
    }

    void Script::rotate_left() {
        // Rotates the image left by 90º degrees
        Image* new_image = new Image(image->height(), image->width(), Color());
    
        for (int h = 0; h < image->height(); h++) {
            for (int w = 0; w < image->width(); w++) {
                Color pixel = image->at(w, h);
                new_image->at(h, image->width() - 1 - w) = pixel;
            }
        }
        
        delete image;
        image = new_image;
    }

    void Script::rotate_right() {
        // Rotates the image right by 90º degrees
        Image* new_image = new Image(image->height(), image->width());
        for (int h = 0; h < image->height(); h++) {
            for (int w = 0; w < image->width(); w++) {
                Color& pixel = image->at(w, h);
                new_image->at(image->height() - 1 - h, w) = pixel;
            }
        }
        delete image;
        image = new_image;
    }

    void Script::median_filter(int ws) {
        //copiar as dimensões da imagem base
        Image* newimage = new Image(image->width(), image->height());

        //deve-se aplicar o processo a todos os pontos, através da seguinte iteração
        for (int x = 0; x < image->width(); x++) {
            for (int y = 0; y < image->height(); y++) {

                int xmin = std::max(0, x - ws / 2);
                int xmax = std::min(image->width() - 1, x + ws / 2);
                int ymin = std::max(0, y - ws / 2);
                int ymax = std::min(image->height() - 1, y + ws /2);

                std::vector<rgb_value> vred, vgreen, vblue; //guardam os valores de red, green e blue, respetivamente, de todos os pontos pertencentes à janela

                for (int nx = xmin; nx <= xmax; nx++) {
                    for (int ny = ymin; ny <= ymax; ny++) {
                        vred.push_back(image->at(nx, ny).red());
                        vgreen.push_back(image->at(nx, ny).green());
                        vblue.push_back(image->at(nx, ny).blue());   
                    }
                }

                //os vetores precisam de estar ordenados para que seja possivel encontrar o valor mediano
                std::sort(vred.begin(), vred.end());
                std::sort(vgreen.begin(), vgreen.end());
                std::sort(vblue.begin(), vblue.end());

                int n = vred.size(); //n é o número de pontos na janela

                rgb_value mred, mgreen, mblue; //representam os valores medianos de cada cor
                

                if (n % 2 != 0) {
                    //caso o número de pontos na janela seja ímpar, a mediana é o elemento central do vetor
                    mred = vred[n/2];
                    mgreen= vgreen[n/2];
                    mblue = vblue[n/2];                    
                }

                else {
                    //caso o número de pontos na janela seja par, a mediana é a média dos dois pontos centrais
                    mred = (vred[n/2] + vred[n/2 - 1]) / 2;
                    mgreen = (vgreen[n/2] + vgreen[n/2 - 1]) / 2;
                    mblue = (vblue[n/2] + vblue[n/2 - 1]) / 2;
                }

                newimage->at(x, y) = Color(mred, mgreen, mblue); //associa a cor definida pelos medianos ao ponto 

            }
        }
        
        //por fim, substituiu-se a imagem
        delete image;
        image = newimage;
    }

}
