// Renders the adaptive musician to a WAV file without a sound card, to listen to a performer
// or to time it. Usage:
//   goe-musician out.wav [seconds=60] [seed=1] [difficulty=128|from:to] [situation=calm|alert|danger] [rate=44100]
// A "from:to" difficulty sweeps linearly over the whole render.
#include "adaptiveMusician.h"
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {
void put32(std::ofstream &o, std::uint32_t v)
{
    o.put((char) (v & 0xff)).put((char) ((v >> 8) & 0xff)).put((char) ((v >> 16) & 0xff)).put((char) (v >> 24));
}
void put16(std::ofstream &o, std::uint16_t v)
{
    o.put((char) (v & 0xff)).put((char) (v >> 8));
}
} // namespace

int main(int argc, char *argv[])
{
    using namespace goe::musician;
    if (argc < 2) {
        std::cerr << "usage: goe-musician out.wav [seconds] [seed] [difficulty|from:to] [calm|alert|danger] [rate]\n"
                     "environment: GOE_STYLE=adlib|sid|pokey|gameboy, GOE_VARIETY=0..1, GOE_TEMPO=0.5..1.5,\n"
                     "             GOE_PHRASES=1 prints a line per phrase\n";
        return 1;
    }
    const double seconds = argc > 2 ? std::atof(argv[2]) : 60.0;
    const std::uint64_t seed = argc > 3 ? std::strtoull(argv[3], nullptr, 10) : 1;
    int from = 128, to = 128;
    if (argc > 4) {
        const std::string d = argv[4];
        const auto colon = d.find(':');
        from = std::atoi(d.substr(0, colon).c_str());
        to = colon == std::string::npos ? from : std::atoi(d.substr(colon + 1).c_str());
    }
    situation s = situation::calm;
    if (argc > 5)
        s = std::strcmp(argv[5], "danger") == 0 ? situation::danger
            : std::strcmp(argv[5], "alert") == 0 ? situation::alert
                                                 : situation::calm;
    const int rate = argc > 6 ? std::atoi(argv[6]) : 44100;

    adaptiveMusician m;
    if (!m.initialize({rate, 2}, seed)) {
        std::cerr << "unsupported format\n";
        return 1;
    }
    m.setDifficulty(from);
    m.setSituation(s);
    if (const char *style = std::getenv("GOE_STYLE"))
        m.setStyle(styleNamed(style));
    if (const char *v = std::getenv("GOE_VARIETY"))
        m.setVariety((float) std::atof(v));
    if (const char *t = std::getenv("GOE_TEMPO"))
        m.setTempoScale((float) std::atof(t));
    const auto total = (std::uint32_t) (seconds * rate);
    const std::uint32_t block = 1024;
    std::vector<float> buf(block * 2);
    std::vector<std::int16_t> pcm;
    pcm.reserve((std::size_t) total * 2);
    const auto t0 = std::chrono::steady_clock::now();
    for (std::uint32_t done = 0; done < total; done += block) {
        m.setDifficulty(from + (int) ((double) (to - from) * done / total));
        const std::uint32_t n = std::min(block, total - done);
        const int before = m.phrasesComposed();
        m.composeAhead();
        if (std::getenv("GOE_PHRASES") && m.phrasesComposed() != before) {
            const auto &r = m.lastPhrase();
            static const char *made[] = {"new", "repeat", "variation"};
            static const char *parts[] = {"intro", "verse", "chorus", "break", "outro"};
            std::printf("%6.1fs song %2u %-6s drums %d%s %-9s motif %3u/%3u bars %d notes/beat %.2f sync %2d chrom %d colour %d/%d leap %2d poly %d tempo %.1f%s\n",
                        (double) done / rate, r.song, parts[(int) r.part], r.drums, r.fill ? "+fill" : "     ",
                        made[(int) r.made], r.motifId, r.family, r.bars, r.notesPerBeat(), r.syncopated, r.chromatic,
                        r.colouredChords, r.chords, r.maxLeap, r.maxSimultaneous, r.tempo,
                        r.silent ? " (lead rests)" : "");
        }
        m.renderAudio(buf.data(), n);
        for (std::uint32_t i = 0; i < n * 2; i++)
            pcm.push_back((std::int16_t) (buf[i] * 32767.0f));
    }
    const double took = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();

    std::ofstream o(argv[1], std::ios::binary);
    const std::uint32_t bytes = (std::uint32_t) (pcm.size() * 2);
    o.write("RIFF", 4);
    put32(o, 36 + bytes);
    o.write("WAVEfmt ", 8);
    put32(o, 16);
    put16(o, 1);
    put16(o, 2);
    put32(o, (std::uint32_t) rate);
    put32(o, (std::uint32_t) rate * 4);
    put16(o, 4);
    put16(o, 16);
    o.write("data", 4);
    put32(o, bytes);
    o.write(reinterpret_cast<const char *>(pcm.data()), bytes);

    const auto &p = m.personality();
    std::cout << nameOf(m.style()) << ", seed " << seed << ": tempo " << p.baseTempo << ", energy " << p.energy << ", complexity "
              << p.rhythmicComplexity << ", dissonance " << p.dissonance << "\n"
              << m.phrasesComposed() << " phrases in " << m.songs().songsStarted() << " songs ("
              << m.songs().songsReturned() << " came back), peak " << m.synth().report().peak << ", rendered " << seconds
              << " s in " << took << " s (" << seconds / took << "x real time)\n";
    return 0;
}
