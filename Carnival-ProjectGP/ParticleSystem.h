#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>

class ParticleSystem {
private:
    std::vector<glm::vec3> particles;
    std::vector<glm::vec3> velocities;
    std::vector<float> lifetimes;
    int particleCount;
    float maxLifetime;
    GLuint VAO, VBO;

public:
    ParticleSystem(int count = 1000, float life = 5.0f)
        : particleCount(count), maxLifetime(life) {

        srand(static_cast<unsigned>(time(0)));

        particles.resize(count);
        velocities.resize(count);
        lifetimes.resize(count);

        // Initialize particles
        for (int i = 0; i < count; ++i) {
            particles[i] = glm::vec3(
                (rand() % 100 - 50) / 10.0f,
                (rand() % 100) / 10.0f,
                (rand() % 100 - 50) / 10.0f
            );

            velocities[i] = glm::vec3(
                (rand() % 10 - 5) / 10.0f,
                -(rand() % 20) / 10.0f,
                (rand() % 10 - 5) / 10.0f
            );

            lifetimes[i] = maxLifetime;
        }

        // Setup VAO/VBO
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, particles.size() * sizeof(glm::vec3), particles.data(), GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    ~ParticleSystem() {
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }

    void update(float deltaTime) {
        for (int i = 0; i < particleCount; ++i) {
            particles[i] += velocities[i] * deltaTime;
            lifetimes[i] -= deltaTime;

            // Recycle particle if expired or fallen too low
            if (particles[i].y < -10.0f || lifetimes[i] <= 0.0f) {
                particles[i] = glm::vec3(
                    (rand() % 100 - 50) / 10.0f,
                    10.0f,
                    (rand() % 100 - 50) / 10.0f
                );

                velocities[i] = glm::vec3(
                    (rand() % 10 - 5) / 10.0f,
                    -(rand() % 20) / 10.0f,
                    (rand() % 10 - 5) / 10.0f
                );

                lifetimes[i] = maxLifetime;
            }
        }

        // Update VBO
        glBindBuffer(GL_COPY_WRITE_BUFFER, VBO);
        glBufferSubData(GL_COPY_WRITE_BUFFER, 0, particles.size() * sizeof(glm::vec3), particles.data());
    }

    void render() {
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, particleCount);
        glBindVertexArray(0);
    }

    GLuint getVAO() const { return VAO; }
    int getParticleCount() const { return particleCount; }
    void setMaxLifetime(float life) { maxLifetime = life; }
};

#endif