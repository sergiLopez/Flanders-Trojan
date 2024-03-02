// ddos.h
#ifndef DDOS_H
#define DDOS_H

#include <string>
#include <vector>
#include <thread>

// Prototipos de funciones para realizar solicitudes
void make_request(const std::string& ip, int port, const std::string& resource);
void executeDDoS();

#endif // DDOS_H
