#pragma once
#include <vector>
#include <string>
#include <iostream>
#include "Memory.hpp"
#include "Offsets.hpp"
#include "Math.hpp"

struct PlayerEntity {
    uintptr_t playerInstance;   // Player instance address
    uintptr_t character;        // Character model
    uintptr_t humanoid;         // Humanoid instance
    uintptr_t rootPart;         // HumanoidRootPart
    uintptr_t head;             // Head part

    std::string name;
    std::string displayName;
    float health;
    float maxHealth;
    Vector3 position;
    Vector3 headPosition;
    Vector3 rootPosition;
    int team;
    bool isAlive;
    bool isLocalPlayer;
    float distance;
};

namespace Entities {
    std::vector<PlayerEntity> players;
    PlayerEntity localPlayer;
    uintptr_t dataModel = 0;
    uintptr_t visualEngine = 0;
    Matrix4x4 viewMatrix;
    int screenWidth = 1920;
    int screenHeight = 1080;

    // Read a Roblox string (Name/DisplayName)
    std::string ReadInstanceName(uintptr_t instance) {
        if (!instance) return "";
        uintptr_t namePtr = Memory::Read<uintptr_t>(instance + offsets::Instance::Name);
        if (!namePtr) return "";
        int nameLen = Memory::Read<int>(namePtr + offsets::Instance::NameSize);
        if (nameLen <= 0 || nameLen > 100) nameLen = 50;
        return Memory::ReadString(namePtr, nameLen);
    }

    // Get the DataModel from TaskScheduler
    uintptr_t GetDataModel() {
        uintptr_t taskScheduler = Memory::Read<uintptr_t>(Memory::baseAddress + offsets::Pointer::TaskScheduler);
        if (!taskScheduler) return 0;

        // TaskScheduler -> Jobs -> RenderJob
        uintptr_t jobsPtr = Memory::Read<uintptr_t>(taskScheduler + offsets::Pointer::JobsPointer);
        if (!jobsPtr) return 0;

        // Iterate through jobs to find RenderJob
        uintptr_t jobStart = Memory::Read<uintptr_t>(taskScheduler + offsets::TaskScheduler::JobStart);
        uintptr_t jobEnd = Memory::Read<uintptr_t>(taskScheduler + offsets::TaskScheduler::JobEnd);

        if (!jobStart || !jobEnd) return 0;

        for (uintptr_t job = jobStart; job != jobEnd; job = Memory::Read<uintptr_t>(job)) {
            if (!job) break;
            uintptr_t jobNamePtr = Memory::Read<uintptr_t>(job + offsets::Jobs::JobName);
            if (!jobNamePtr) continue;
            std::string jobName = Memory::ReadString(jobNamePtr, 30);

            if (jobName.find("Render") != std::string::npos) {
                // Found RenderJob -> get FakeDataModel -> DataModel
                uintptr_t fakeDM = Memory::Read<uintptr_t>(job + offsets::RenderJob::FakeDataModel);
                if (!fakeDM) continue;
                uintptr_t dm = Memory::Read<uintptr_t>(fakeDM + offsets::FakeDataModel::DataModel);
                if (dm) return dm;
            }
        }

        return 0;
    }

    // Get VisualEngine from DataModel
    uintptr_t GetVisualEngine(uintptr_t dm) {
        if (!dm) return 0;

        // DataModel -> RenderView -> VisualEngine
        uintptr_t renderView1 = Memory::Read<uintptr_t>(dm + offsets::DataModel::DataModelToRenderView1);
        if (!renderView1) return 0;
        uintptr_t renderView2 = Memory::Read<uintptr_t>(renderView1 + offsets::DataModel::DataModelToRenderView2);
        if (!renderView2) return 0;
        uintptr_t renderView3 = Memory::Read<uintptr_t>(renderView2 + offsets::DataModel::DataModelToRenderView3);
        if (!renderView3) return 0;

        uintptr_t ve = Memory::Read<uintptr_t>(renderView3 + offsets::RenderView::VisualEngine);
        return ve;
    }

    // Read view matrix from VisualEngine
    Matrix4x4 ReadViewMatrix(uintptr_t ve) {
        Matrix4x4 mat;
        if (!ve) return mat;
        Memory::ReadBuffer(ve + offsets::VisualEngine::viewmatrix, &mat, sizeof(Matrix4x4));
        return mat;
    }

    // Read screen dimensions from VisualEngine
    void ReadScreenDimensions(uintptr_t ve) {
        if (!ve) return;
        Vector2 dims = Memory::Read<Vector2>(ve + offsets::VisualEngine::Dimensions);
        if (dims.x > 100 && dims.y > 100) {
            screenWidth = (int)dims.x;
            screenHeight = (int)dims.y;
        }
    }

    // Get the Players service from DataModel
    uintptr_t GetPlayersService(uintptr_t dm) {
        if (!dm) return 0;
        uintptr_t workspace = Memory::Read<uintptr_t>(dm + offsets::DataModel::Workspace);
        if (!workspace) return 0;

        // Walk children of Workspace to find "Players"
        uintptr_t children = Memory::Read<uintptr_t>(workspace + offsets::Instance::Children);
        if (!children) return 0;

        uintptr_t child = Memory::Read<uintptr_t>(children);
        while (child) {
            std::string childName = ReadInstanceName(child);
            if (childName == "Players") {
                return child;
            }
            child = Memory::Read<uintptr_t>(child + offsets::Instance::ChildrenEnd);
        }

        return 0;
    }

    // Get local player from Players service
    uintptr_t GetLocalPlayer(uintptr_t playersService) {
        if (!playersService) return 0;
        return Memory::Read<uintptr_t>(playersService + offsets::Players::LocalPlayer);
    }

    // Get character from player
    uintptr_t GetCharacter(uintptr_t player) {
        if (!player) return 0;
        return Memory::Read<uintptr_t>(player + offsets::Player::ModelInstance);
    }

    // Get humanoid from character
    uintptr_t GetHumanoid(uintptr_t character) {
        if (!character) return 0;
        // Walk children to find Humanoid
        uintptr_t children = Memory::Read<uintptr_t>(character + offsets::Instance::Children);
        if (!children) return 0;

        uintptr_t child = Memory::Read<uintptr_t>(children);
        while (child) {
            std::string childName = ReadInstanceName(child);
            if (childName == "Humanoid") {
                return child;
            }
            child = Memory::Read<uintptr_t>(child + offsets::Instance::ChildrenEnd);
        }
        return 0;
    }

    // Get root part from character
    uintptr_t GetRootPart(uintptr_t character) {
        if (!character) return 0;
        // Walk children to find HumanoidRootPart
        uintptr_t children = Memory::Read<uintptr_t>(character + offsets::Instance::Children);
        if (!children) return 0;

        uintptr_t child = Memory::Read<uintptr_t>(children);
        while (child) {
            std::string childName = ReadInstanceName(child);
            if (childName == "HumanoidRootPart") {
                return child;
            }
            child = Memory::Read<uintptr_t>(child + offsets::Instance::ChildrenEnd);
        }
        return 0;
    }

    // Get head part from character
    uintptr_t GetHead(uintptr_t character) {
        if (!character) return 0;
        uintptr_t children = Memory::Read<uintptr_t>(character + offsets::Instance::Children);
        if (!children) return 0;

        uintptr_t child = Memory::Read<uintptr_t>(children);
        while (child) {
            std::string childName = ReadInstanceName(child);
            if (childName == "Head") {
                return child;
            }
            child = Memory::Read<uintptr_t>(child + offsets::Instance::ChildrenEnd);
        }
        return 0;
    }

    // Get position from a BasePart
    Vector3 GetPartPosition(uintptr_t part) {
        if (!part) return Vector3();
        return Memory::Read<Vector3>(part + offsets::BasePart::Position);
    }

    // Get all players
    void UpdateEntities() {
        players.clear();

        // Get DataModel
        dataModel = GetDataModel();
        if (!dataModel) {
            // Fallback: try direct pointer
            dataModel = Memory::Read<uintptr_t>(Memory::baseAddress + offsets::Pointer::DataModelDeleterPointer);
            if (dataModel) {
                dataModel = Memory::Read<uintptr_t>(dataModel + offsets::Instance::Deleter);
            }
        }
        if (!dataModel) return;

        // Get VisualEngine
        visualEngine = GetVisualEngine(dataModel);
        if (!visualEngine) {
            // Fallback
            visualEngine = Memory::Read<uintptr_t>(Memory::baseAddress + offsets::Pointer::VisualEnginePointer);
        }
        if (visualEngine) {
            viewMatrix = ReadViewMatrix(visualEngine);
            ReadScreenDimensions(visualEngine);
        }

        // Get Players service
        uintptr_t playersService = GetPlayersService(dataModel);
        if (!playersService) return;

        // Get local player
        uintptr_t localPlayerPtr = GetLocalPlayer(playersService);
        if (!localPlayerPtr) return;

        // Read local player info
        localPlayer.playerInstance = localPlayerPtr;
        localPlayer.name = ReadInstanceName(localPlayerPtr);
        localPlayer.displayName = Memory::ReadString(localPlayerPtr + offsets::Player::DisplayName, 30);
        localPlayer.team = Memory::Read<int>(localPlayerPtr + offsets::Player::Team);
        localPlayer.isLocalPlayer = true;

        uintptr_t localChar = GetCharacter(localPlayerPtr);
        if (localChar) {
            localPlayer.character = localChar;
            localPlayer.rootPart = GetRootPart(localChar);
            if (localPlayer.rootPart) {
                localPlayer.rootPosition = GetPartPosition(localPlayer.rootPart);
            }
        }

        // Iterate through children of Players service to find all players
        uintptr_t children = Memory::Read<uintptr_t>(playersService + offsets::Instance::Children);
        if (!children) return;

        uintptr_t child = Memory::Read<uintptr_t>(children);
        while (child) {
            std::string childName = ReadInstanceName(child);

            // Check if this is a Player (not a folder/service)
            if (!childName.empty() && child != localPlayerPtr) {
                // Check if it has a ModelInstance (character) - indicates it's a player
                uintptr_t modelInst = Memory::Read<uintptr_t>(child + offsets::Player::ModelInstance);
                if (modelInst) {
                    PlayerEntity entity;
                    entity.playerInstance = child;
                    entity.name = childName;
                    entity.displayName = Memory::ReadString(child + offsets::Player::DisplayName, 30);
                    entity.team = Memory::Read<int>(child + offsets::Player::Team);
                    entity.isLocalPlayer = false;
                    entity.character = modelInst;

                    // Get humanoid
                    entity.humanoid = GetHumanoid(modelInst);
                    if (entity.humanoid) {
                        entity.health = Memory::Read<float>(entity.humanoid + offsets::Humanoid::Health);
                        entity.maxHealth = Memory::Read<float>(entity.humanoid + offsets::Humanoid::MaxHealth);
                        entity.isAlive = (entity.health > 0);
                    } else {
                        entity.health = 0;
                        entity.maxHealth = 100;
                        entity.isAlive = false;
                    }

                    // Get root part
                    entity.rootPart = GetRootPart(modelInst);
                    if (entity.rootPart) {
                        entity.rootPosition = GetPartPosition(entity.rootPart);
                    }

                    // Get head
                    entity.head = GetHead(modelInst);
                    if (entity.head) {
                        entity.headPosition = GetPartPosition(entity.head);
                    } else if (entity.rootPart) {
                        // Approximate head position
                        entity.headPosition = entity.rootPosition;
                        entity.headPosition.y += 3.0f; // Approximate head height
                    }

                    // Calculate distance
                    if (localPlayer.rootPart) {
                        entity.position = entity.rootPosition;
                        entity.distance = localPlayer.rootPosition.Distance(entity.rootPosition);
                    }

                    players.push_back(entity);
                }
            }

            child = Memory::Read<uintptr_t>(child + offsets::Instance::ChildrenEnd);
        }

        std::cout << "[+] Found " << players.size() << " players" << std::endl;
    }
}