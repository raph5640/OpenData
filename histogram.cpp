#include "histogram.h"
#include <iostream>
#include <fstream>
#include <string>
#include <gd.h>
#include <nlohmann/json.hpp>
#include <vector>
#include <cmath>
#include <map>
#include <gdfonts.h>
#include <curl/curl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctime>
#include <iomanip>
using namespace std;
//Auteur : Raphael De Oliveira

vector<string> split_into_lines(const string& s) {
    istringstream iss(s);
    vector<string> lines;
    string line;
    while (getline(iss, line)) {
        lines.push_back(line);
    }
    return lines;
}
// Fonction pour créer un histogramme
void Histogram::createHistogram(const string& filename, const vector<string>& noms, const vector<int>& dispo, const vector<int>& max) {
    const int image_width = 1600;
    const int image_height = 600;
    const int bar_width = image_width / noms.size();
    const int MAX_BAR_HEIGHT = image_height - 20;

    gdImagePtr im = gdImageCreateTrueColor(image_width, image_height);
    int white = gdImageColorAllocate(im, 255, 255, 255);
    gdImageFill(im, 0, 0, white);

    int black = gdImageColorAllocate(im, 0, 0, 0);  // Couleur pour le texte

    //Dessin des barres de l'histogramme
    for (size_t i = 0; i < dispo.size(); i++) {
        int pourcentage = (100 * dispo[i]) / max[i];
        int bar_height = (image_height * pourcentage) / 100;
        const int RELIEF_OFFSET = 10;
        bar_height = min(bar_height, MAX_BAR_HEIGHT);
        int rouge = rand() % 255;
        int vert = rand() % 255;
        int bleu = rand() % 255;
        int color = gdImageColorAllocate(im, rouge, vert, bleu);
        int color_relief = gdImageColorAllocate(im, rouge-30, vert, bleu);
        // Dessine le rectangle principal
        gdImageFilledRectangle(im, i * bar_width, image_height - bar_height, (i + 1) * bar_width - RELIEF_OFFSET, image_height, color);
        //On dessine le reflief 3D
        for (int offset = 1; offset <= RELIEF_OFFSET; offset++) {
            gdImageFilledRectangle(im,(i + 1) * bar_width - offset,image_height - bar_height - offset + RELIEF_OFFSET,(i + 1) * bar_width - offset + 1,image_height - offset + RELIEF_OFFSET,color_relief);
        }

        string label = noms[i] + " (" + to_string(pourcentage) + "%)";
        if (pourcentage < 90) {     //Si le pourcentage du parking est inferieur a 90% on écrit son nom sur plusieurs ligne pour éviter les cheuvauchements du texte sur les autres barres suivantes
            auto lines = split_into_lines(label);
            int offsetY = (lines.size() - 1) * 12;
            for (const auto& line : lines) {
                gdImageString(im, gdFontGetSmall(), i * bar_width + 5, image_height - bar_height - 15 - offsetY, (unsigned char*)line.c_str(), black);
                offsetY -= 12;  //décrémente la valeur pour aller vers le haut
            }
        } else {  //sinon si le pourcentage est tres haut (>90) on considerer que si l'on affiche le nom entier du parking sur une ligne ca ne pose pas de probleme d'affichage et de lisibilité
            gdImageString(im, gdFontGetSmall(), i * bar_width + 5, image_height - bar_height - 15, (unsigned char*)label.c_str(), black);
        }
    }

    FILE* out = fopen(filename.c_str(), "wb");
    gdImagePng(im, out);
    fclose(out);

    gdImageDestroy(im);
}


void Histogram::createEvolutionHistogram(const string& filename, const vector<string>& timestamps, const vector<int>& data) {
    const int image_width = 1600;
    const int image_height = 600;
    const int bar_width = image_width / data.size();
    const int MAX_BAR_HEIGHT = image_height - 20;

    gdImagePtr im = gdImageCreateTrueColor(image_width, image_height);
    int white = gdImageColorAllocate(im, 255, 255, 255);
    gdImageFill(im, 0, 0, white);

    int black = gdImageColorAllocate(im, 0, 0, 0);  // Couleur pour le texte

    for (size_t i = 0; i < data.size(); i++) {
        int pourcentage = data[i];
        int bar_height = (image_height * pourcentage) / 100;
        bar_height = min(bar_height, MAX_BAR_HEIGHT);
        int rouge = rand() % 255;
        int vert = rand() % 255;
        int bleu = rand() % 255;
        int color = gdImageColorAllocate(im, rouge, vert, bleu);

        gdImageFilledRectangle(im, i * bar_width, image_height - bar_height, (i + 1) * bar_width, image_height, color);

        //Écris un label en haut à gauche de chaque barre indiquant le pourcentage
        string label = to_string(pourcentage) + "%";
        gdImageString(im, gdFontGetSmall(), i * bar_width + 5, image_height - bar_height - 15, (unsigned char*)label.c_str(), black);

        //On obtient la date et l'heure à partir du timestamp
        string timestamp = timestamps[i];
        size_t pos = timestamp.find(' ');
        if (pos != string::npos) {
            string dateStr = timestamp.substr(0, pos);
            string heureStr = timestamp.substr(pos + 1);

            int textOffset = (bar_width - gdFontGetSmall()->w * (dateStr.length() + heureStr.length())) / 2;
            gdImageString(im, gdFontGetSmall(), i * bar_width + textOffset, image_height - 30, (unsigned char*)dateStr.c_str(), black);
            gdImageString(im, gdFontGetSmall(), i * bar_width + textOffset, image_height - 15, (unsigned char*)heureStr.c_str(), black);
        }
    }

    FILE* out = fopen(filename.c_str(), "wb");
    gdImagePng(im, out);
    fclose(out);

    gdImageDestroy(im);
}
