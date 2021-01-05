//ParticleSystem.h
#pragma once

#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <random>
#include "shader.h"
#include <ctime>

float gravity = 9.8f;

struct Particle
{
	glm::vec3 speed = glm::vec3(0.0f);
	glm::vec3 pos = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(0.0f);
	float lifetime = 5.0f;
	Particle(glm::vec3 apos, glm::vec3 aspeed, glm::vec3 ascale) {
		pos = apos;
		speed = aspeed;
		scale = ascale;
	}
};

class ParticleSystem
{
public:
	std::vector<Particle> particles;
	ParticleSystem();
	~ParticleSystem();

	void create_particles(glm::vec3 knife_pos, float mount);
	void destroy_particles(int index);
	void update(float deltaTime);
	void draw_particles(Shader shader, unsigned int VAO, glm::mat4 projection, glm::mat4 view, glm::vec3 lightPos);
private:

};

ParticleSystem::ParticleSystem()
{
	srand(unsigned(time(NULL)));
	//std::cout << "load particlesystem" << std::endl;
	//Particle tmp(glm::vec3(1.0f), glm::vec3(1.0f), glm::vec3(1.0f));
	//particles.push_back(tmp);
}

ParticleSystem::~ParticleSystem()
{
	particles.clear();
}

inline void ParticleSystem::create_particles(glm::vec3 knife_pos, float mount)
{
	//std::cout << "create particlesystem mount:" << mount << std::endl;
	if (mount==0.0f)
	{
		return;
	}
	int amount = mount * 2;
	if (amount<1)
	{
		amount = 1;//保底生成一个方块
	}
	for (int i = 0; i < amount; i++)
	{
		float fnum = (rand() % 2000) / 1000.0 - 1; //产生-1~1的浮点数
		//std::cout << "create particlesystem" << std::endl;
		glm::vec3 speed = glm::vec3(fnum * 3.0f, fnum * 2.0f, -fnum * 4.0 - 6.0f);
		Particle tmp(knife_pos, speed, glm::vec3(mount*0.3f));
		particles.push_back(tmp);
	}
}

inline void ParticleSystem::destroy_particles(int index)
{
}

inline void ParticleSystem::update(float deltaTime)
{
	if (particles.size() == 0)
	{
		return;
	}

	for (int i = 0; i < particles.size(); i++){
		particles[i].lifetime -= deltaTime;
		particles[i].pos += particles[i].speed * deltaTime;
		particles[i].speed.y -= gravity * deltaTime;
		//std::cout << particles[i].lifetime << std::endl;
		if (particles[i].lifetime<=0.0f)
		{
			particles.erase(particles.begin() + i);
			//std::cout << "erase particle " << std::endl;
			i--;
		}
	}
}

inline void ParticleSystem::draw_particles(Shader shader, unsigned int VAO, glm::mat4 projection, glm::mat4 view, glm::vec3 lightPos)
{
	for (int i = 0; i < particles.size(); i++)
	{
		//std::cout << "draw particle index: " << i << std::endl;
		// particle render
		//shader.use();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, particles[i].pos);
		model = glm::rotate(model, 5*(float)glfwGetTime(), glm::vec3(1.0f, 1.0f, 1.0f));//x轴控制轴心自转
		model = glm::scale(model, particles[i].scale); // a smaller cube
		shader.setMat4("model", model);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
}


#endif