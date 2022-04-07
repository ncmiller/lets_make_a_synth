#pragma once

struct Synth;

class Input {
public:
    bool Init(Synth* synth) {
        _synth = synth;
        return true;
    }
    void PollEvents();
private:
    Synth* _synth; // parent
};
