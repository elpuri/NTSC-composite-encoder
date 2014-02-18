#include <QCoreApplication>
#include <QImage>
#include <QDebug>
#include <QFile>
#include <QColor>
#include <QTextStream>

QString intToBin(int a, int width) {
    QString bin;
    for (int i = 0; i < width; i++) {
        bin.prepend(a & 1 ? "1" : "0");
        a = a >> 1;
    }
    return bin;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (argc < 4) {
        qDebug() << "Usage imgpacker <image file> <image mif file> <palette vhdl file>";
        return 0;
    }

    QImage source(a.arguments().at(1));
    if (source.isNull()) {
        qDebug() << "Couldn't open input image" << a.arguments().at(1);
    }

    if (source.format() != QImage::Format_Indexed8) {
        qDebug() << "Pixel format must be indexed color";
        return 0;
    }

    if (source.colorCount() > 16) {
        qDebug() << "Color count must be <= 16. The source file has" << source.colorCount() << "colors.";
        return 0;
    }

    if (source.width() & 1) {
        qDebug() << "Image width must be an even number";
        return 0;
    }

    QFile target(a.arguments().at(2));
    if (!target.open(QFile::WriteOnly)) {
        qDebug() << "Couldn't open target file" << a.arguments().at(2);
        return 0;
    }

    // Pack two 4-bit pixels into one byte
    // Wider word is cheaper on the FPGA when synthesizing the ROM
    QByteArray data;
    data.reserve(source.width() * source.height() / 2);
    for (int y = 0; y < source.height(); y++) {
        for (int x = 0; x < source.width(); x += 2) {
            unsigned char b = source.pixelIndex(x, y) << 4;
            b |= source.pixelIndex(x + 1, y);
            data.append(b);
        }
    }

    target.write(data);
    target.close();

    // Generate palette
    QFile paletteFile(a.arguments().at(3));
    if (!paletteFile.open(QFile::WriteOnly));

    QTextStream output(&paletteFile);
    output << "-- Copyright (c) 2014, Juha Turunen" << endl;
    output << "-- All rights reserved." << endl;
    output << "--" << endl;
    output << "-- Redistribution and use in source and binary forms, with or without" << endl;
    output << "-- modification, are permitted provided that the following conditions are met: " << endl;
    output << "--" << endl;
    output << "-- 1. Redistributions of source code must retain the above copyright notice, this" << endl;
    output << "--    list of conditions and the following disclaimer. " << endl;
    output << "-- 2. Redistributions in binary form must reproduce the above copyright notice," << endl;
    output << "--    this list of conditions and the following disclaimer in the documentation" << endl;
    output << "--    and/or other materials provided with the distribution. " << endl;
    output << "--" << endl;
    output << "-- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND" << endl;
    output << "-- ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED" << endl;
    output << "-- WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE" << endl;
    output << "-- DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR" << endl;
    output << "-- ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES" << endl;
    output << "-- (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;" << endl;
    output << "-- LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND" << endl;
    output << "-- ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT" << endl;
    output << "-- (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS" << endl;
    output << "-- SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE." << endl;
    output << endl;
    output << "library IEEE;" << endl;
    output << "use IEEE.STD_LOGIC_1164.ALL;" << endl;
    output << "use IEEE.STD_LOGIC_ARITH.ALL;" << endl;
    output << "use IEEE.STD_LOGIC_UNSIGNED.ALL;" << endl;
    output << endl;
    output << "entity palette is Port ( " << endl;
    output << "    index : in std_logic_vector(3 downto 0);" << endl;
    output << "    luma : out std_logic_vector(10 downto 0);" << endl;
    output << "    phase : out std_logic_vector(7 downto 0);" << endl;
    output << "    chroma : out std_logic_vector(2 downto 0)" << endl;
    output << " );" << endl;
    output << "end palette;" << endl;
    output << endl;
    output << "architecture Behavioral of palette is" << endl;
    output << "begin" << endl;
    output << endl;

    output << "\tluma <=" << endl;
    QVector<QRgb> colors = source.colorTable();
    int i = 0;
    foreach (QRgb color, colors) {
        QColor c(color);
        float l = c.redF() * 0.299 + c.greenF() * 0.587 + c.blueF() * 0.114;
        int luma = 278 + l * 1023;
        if (i != colors.count() - 1)
            output << "\t\tconv_std_logic_vector(" << luma << ", 11) when index = \"" << intToBin(i, 4) << "\" else" << endl;
        else
            output << "\t\tconv_std_logic_vector(" << luma << ", 11);" << endl;
        i++;
    }

    output << endl;

    i = 0;
    output << "\tchroma <=" << endl;

    foreach (QRgb color, colors) {
        QColor c(color);
        QColor hsv = c.toHsv();
        int chroma = (1.0 - c.saturationF()) * 7;
        if (c.saturationF() < 0.1)
            chroma = 7;
        if (i != colors.count() - 1)
            output << "\t\t\"" << intToBin(chroma, 3) << "\" when index = \"" << intToBin(i, 4) << "\" else" << endl;
        else
            output << "\t\t\"" << intToBin(chroma, 3) << "\";" << endl;
        i++;
    }


    i = 0;
    output << "\tphase <=" << endl;

    foreach (QRgb color, colors) {
        QColor c(color);
        float hue = c.hslHueF() - 0.166;
        if (hue > 0.5) hue -= 1.0;
        int phase = hue * 255.0;

        if (i != colors.count() - 1)
            output << "\t\t\"" << intToBin(phase, 8) << "\" when index = \"" << intToBin(i, 4) << "\" else" << endl;
        else
            output << "\t\t\"" << intToBin(phase, 8) << "\";" << endl;
        i++;
    }

    output << endl;
    output << "end Behavioral;" << endl;

    output.flush();
    paletteFile.close();
}

