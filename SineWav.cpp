#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <limits>

//Great resource! - http://soundfile.sapp.org/doc/WaveFormat/

const int sampleRate = 48000;
const int bitDepth = 24;
const int numChannels = 1;

const int maxAmplitude = pow(2, bitDepth - 1) - 1;
const int maxLengthInSeconds = 4 * pow(10, 9) / sampleRate / 4 - 1;//Wav files can't be > 4GB.


class SineWave
{
    float frequency;
    float amplitude;
    float angle = 0.0f;
    float offset = 0.0f;

public:
    SineWave(float freq, float amp) : frequency(freq), amplitude(amp)
    {
        //Precalculate angle value
        offset = 2 * M_PI * frequency / sampleRate;
    }

    //Make the sine wave, return a float value "sample"
    float processSample()
    {
        //Sine wave formula: amplitude * sin(2 * M_PI * frequency / sampleRate)
        //Angle of the sine wave (2* M_PI * frequency / sampleRate) must be incremented each sample

        auto sample = amplitude * sin(angle);
        angle += offset;
        return sample;
    }
};

//Helper method -- make sure ints get saved with the right amount of bytes
//might be necessary to use fopen and fwrite instead of ofstream
void byteWrite(std::ofstream& file, int value, int numBytes)
{
    file.write(reinterpret_cast<const char*>(&value), numBytes);
}

void generateWavFile(float frequency, float lengthInSeconds)
{
    if (lengthInSeconds < 0 || frequency < 0)
        throw std::runtime_error("Cannot use negative numbers.");
    if (lengthInSeconds > maxLengthInSeconds)
        throw std::runtime_error("Maximum number of seconds exceeded.");
    if (frequency > 24000)
        throw std::runtime_error("Maximum frequency exceeded.");

    std::ofstream outFile;
//    FILE* outFile = fopen("outputAudio.wav", "wb");
    outFile.open("outputAudio.wav", std::ios::binary);

    SineWave sineWave(frequency, 0.2);//sineWave object with amplitude of 0.2 (to keep your ears safe)

    //RIFF chunk
    /*ChunkID      */outFile << "RIFF";
    /*ChunkSize    */byteWrite(outFile, sampleRate * lengthInSeconds * 4, 4);//simpler than using tellp and seekp
    /*Format       */outFile << "WAVE";

    //fmt sub-chunk
    /*Subchunk1ID  */outFile << "fmt ";
    /*Subchunk1Size*/byteWrite(outFile, 16, 4);
    /*AudioFormat  */byteWrite(outFile, 1, 2);
    /*NumChannels  */byteWrite(outFile, 1, 2);
    /*SampleRate   */byteWrite(outFile, sampleRate, 4);
    /*ByteRate     */byteWrite(outFile, sampleRate * numChannels * bitDepth / 8, 4);
    /*BlockAlign   */byteWrite(outFile, numChannels * (bitDepth / 8), 2);
    /*BitsPerSample*/byteWrite(outFile, bitDepth, 2);

    //data sub-chunk
    /*Subchunk2ID  */outFile << "data";
    /*Subchunk2Size*/byteWrite(outFile, sampleRate * lengthInSeconds * 4, 4);

    /*data:        */

    //loop through all samples within specified length
    for (int i = 0; i < sampleRate * lengthInSeconds; i++)
    {
        //Inserting samples into WAV file, line-by-line
        auto sample = sineWave.processSample();
        int intSample = static_cast<int> (sample * maxAmplitude);
        byteWrite(outFile, intSample, bitDepth / 8);
    }

    outFile.close();
}

int main()
{
    float f;
    float t;

    std::cout << "Enter frequency in Hz:" << std::endl;

    std::cin >> f;

    while (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Error: Input is not a number." << std::endl << "Enter frequency in Hz:" << std::endl;
        std::cin >> f;
    }

    std::cout << "Enter length in seconds:" << std::endl;

    std::cin >> t;

    while (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Error: Input is not a number." << std::endl << "Enter length in seconds:" << std::endl;
        std::cin >> t;
    }

    //where exceptions would most often occur
    try
    {
        std::cout << "Generating audio file..." << std::endl;
        generateWavFile(f, t);
        std::cout << "Audio file generated." << std::endl;
    }
    catch (std::exception const& e)
    {
        std::cout << "Error: " << e.what() << std::endl;
    }

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}