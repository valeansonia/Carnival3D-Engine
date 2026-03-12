#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Animator {
private:
    float elapsedTime;
    float duration;
    bool isLooping;

public:
    Animator(float duration = 2.0f, bool looping = true)
        : elapsedTime(0.0f), duration(duration), isLooping(looping) {
    }

    void update(float deltaTime) {
        elapsedTime += deltaTime;
        if (isLooping && elapsedTime > duration) {
            elapsedTime = 0.0f;
        }
    }

    float getProgress() {
        return (duration > 0) ? glm::clamp(elapsedTime / duration, 0.0f, 1.0f) : 0.0f;
    }

    bool isFinished() { return elapsedTime >= duration; }

    void reset() { elapsedTime = 0.0f; }

    float getElapsedTime() const { return elapsedTime; }
    float getDuration() const { return duration; }
    void setDuration(float newDuration) { duration = newDuration; }
    void setLooping(bool loop) { isLooping = loop; }
};

#endif