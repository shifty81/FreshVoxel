#include "character/BodyPartLibrary.h"

#include <cmath>
#include <random>

namespace fresh
{

BodyPartLibrary::BodyPartLibrary()
{
    initialize();
}

BodyPartLibrary::~BodyPartLibrary() = default;

void BodyPartLibrary::initialize()
{
    generateHeadVariations();
    generateTorsoVariations();
    generateLimbVariations();
}

const BodyPartTemplate* BodyPartLibrary::getTemplate(BodyPartType type, int variationIndex) const
{
    auto it = templates.find(type);
    if (it != templates.end() && variationIndex >= 0 &&
        variationIndex < static_cast<int>(it->second.size())) {
        return &it->second[variationIndex];
    }
    return nullptr;
}

int BodyPartLibrary::getVariationCount(BodyPartType type) const
{
    auto it = templates.find(type);
    if (it != templates.end()) {
        return static_cast<int>(it->second.size());
    }
    return 0;
}

std::vector<BodyPartTemplate> BodyPartLibrary::createRandomCharacter(uint32_t seed) const
{
    std::mt19937 rng(seed);
    std::vector<BodyPartTemplate> parts;

    // Pick random variation for each body part type
    std::vector<BodyPartType> partTypes = {BodyPartType::Head,     BodyPartType::Torso,
                                           BodyPartType::LeftArm,  BodyPartType::RightArm,
                                           BodyPartType::LeftLeg,  BodyPartType::RightLeg,
                                           BodyPartType::LeftHand, BodyPartType::RightHand,
                                           BodyPartType::LeftFoot, BodyPartType::RightFoot};

    for (BodyPartType type : partTypes) {
        int count = getVariationCount(type);
        if (count > 0) {
            int variationIndex = rng() % count;
            const BodyPartTemplate* tmpl = getTemplate(type, variationIndex);
            if (tmpl) {
                parts.push_back(*tmpl);
            }
        }
    }

    return parts;
}

void BodyPartLibrary::generateHeadVariations()
{
    std::vector<BodyPartTemplate>& heads = templates[BodyPartType::Head];

    // Variation 1: Cubic head (classic blocky style)
    {
        BodyPartTemplate head;
        head.type = BodyPartType::Head;
        head.dimensions = {4, 4, 4};
        head.attachmentPoint = {0.0f, -2.0f, 0.0f};

        for (int x = -2; x < 2; ++x) {
            for (int y = 0; y < 4; ++y) {
                for (int z = -2; z < 2; ++z) {
                    CharacterVoxel voxel;
                    voxel.position = {x, y, z};
                    voxel.color = {0.8f, 0.6f, 0.5f}; // Skin tone
                    voxel.boneIndex = 3;              // Head bone
                    head.voxels.push_back(voxel);
                }
            }
        }
        heads.push_back(head);
    }

    // Variation 2: Rounded head (spherical)
    {
        BodyPartTemplate head;
        head.type = BodyPartType::Head;
        head.dimensions = {5, 5, 5};
        head.attachmentPoint = {0.0f, -2.0f, 0.0f};

        const float radius = 2.5f;
        for (int x = -2; x <= 2; ++x) {
            for (int y = 0; y <= 4; ++y) {
                for (int z = -2; z <= 2; ++z) {
                    float dist = std::sqrt(static_cast<float>(x * x + (y - 2) * (y - 2) + z * z));
                    if (dist <= radius) {
                        CharacterVoxel voxel;
                        voxel.position = {x, y, z};
                        voxel.color = {0.8f, 0.6f, 0.5f};
                        voxel.boneIndex = 3;
                        head.voxels.push_back(voxel);
                    }
                }
            }
        }
        heads.push_back(head);
    }

    // Variation 3: Tall head (elongated)
    {
        BodyPartTemplate head;
        head.type = BodyPartType::Head;
        head.dimensions = {4, 5, 4};
        head.attachmentPoint = {0.0f, -2.0f, 0.0f};

        for (int x = -2; x < 2; ++x) {
            for (int y = 0; y < 5; ++y) {
                for (int z = -2; z < 2; ++z) {
                    CharacterVoxel voxel;
                    voxel.position = {x, y, z};
                    voxel.color = {0.8f, 0.6f, 0.5f};
                    voxel.boneIndex = 3;
                    head.voxels.push_back(voxel);
                }
            }
        }
        heads.push_back(head);
    }

    // Variation 4: Wide head (broader)
    {
        BodyPartTemplate head;
        head.type = BodyPartType::Head;
        head.dimensions = {5, 4, 4};
        head.attachmentPoint = {0.0f, -2.0f, 0.0f};

        for (int x = -2; x <= 2; ++x) {
            for (int y = 0; y < 4; ++y) {
                for (int z = -2; z < 2; ++z) {
                    CharacterVoxel voxel;
                    voxel.position = {x, y, z};
                    voxel.color = {0.8f, 0.6f, 0.5f};
                    voxel.boneIndex = 3;
                    head.voxels.push_back(voxel);
                }
            }
        }
        heads.push_back(head);
    }

    // Variation 5: Small head (compact)
    {
        BodyPartTemplate head;
        head.type = BodyPartType::Head;
        head.dimensions = {3, 3, 3};
        head.attachmentPoint = {0.0f, -1.5f, 0.0f};

        for (int x = -1; x <= 1; ++x) {
            for (int y = 0; y < 3; ++y) {
                for (int z = -1; z <= 1; ++z) {
                    CharacterVoxel voxel;
                    voxel.position = {x, y, z};
                    voxel.color = {0.8f, 0.6f, 0.5f};
                    voxel.boneIndex = 3;
                    head.voxels.push_back(voxel);
                }
            }
        }
        heads.push_back(head);
    }
}

void BodyPartLibrary::generateTorsoVariations()
{
    std::vector<BodyPartTemplate>& torsos = templates[BodyPartType::Torso];

    // Variation 1: Standard torso (rectangular)
    {
        BodyPartTemplate torso;
        torso.type = BodyPartType::Torso;
        torso.dimensions = {4, 6, 2};
        torso.attachmentPoint = {0.0f, 0.0f, 0.0f};

        for (int x = -2; x < 2; ++x) {
            for (int y = 0; y < 6; ++y) {
                for (int z = -1; z < 1; ++z) {
                    CharacterVoxel voxel;
                    voxel.position = {x, y, z};
                    voxel.color = {0.2f, 0.4f, 0.8f}; // Blue clothing
                    voxel.boneIndex = 1;              // Spine bone
                    torso.voxels.push_back(voxel);
                }
            }
        }
        torsos.push_back(torso);
    }

    // Variation 2: Bulky torso (muscular/armored)
    {
        BodyPartTemplate torso;
        torso.type = BodyPartType::Torso;
        torso.dimensions = {5, 6, 3};
        torso.attachmentPoint = {0.0f, 0.0f, 0.0f};

        for (int x = -2; x <= 2; ++x) {
            for (int y = 0; y < 6; ++y) {
                for (int z = -1; z <= 1; ++z) {
                    CharacterVoxel voxel;
                    voxel.position = {x, y, z};
                    voxel.color = {0.3f, 0.3f, 0.3f}; // Gray armor
                    voxel.boneIndex = 1;
                    torso.voxels.push_back(voxel);
                }
            }
        }
        torsos.push_back(torso);
    }

    // Variation 3: Slim torso (lean build)
    {
        BodyPartTemplate torso;
        torso.type = BodyPartType::Torso;
        torso.dimensions = {3, 6, 2};
        torso.attachmentPoint = {0.0f, 0.0f, 0.0f};

        for (int x = -1; x <= 1; ++x) {
            for (int y = 0; y < 6; ++y) {
                for (int z = -1; z < 1; ++z) {
                    CharacterVoxel voxel;
                    voxel.position = {x, y, z};
                    voxel.color = {0.2f, 0.4f, 0.8f};
                    voxel.boneIndex = 1;
                    torso.voxels.push_back(voxel);
                }
            }
        }
        torsos.push_back(torso);
    }

    // Variation 4: Tall torso (elongated)
    {
        BodyPartTemplate torso;
        torso.type = BodyPartType::Torso;
        torso.dimensions = {4, 7, 2};
        torso.attachmentPoint = {0.0f, 0.0f, 0.0f};

        for (int x = -2; x < 2; ++x) {
            for (int y = 0; y < 7; ++y) {
                for (int z = -1; z < 1; ++z) {
                    CharacterVoxel voxel;
                    voxel.position = {x, y, z};
                    voxel.color = {0.2f, 0.4f, 0.8f};
                    voxel.boneIndex = 1;
                    torso.voxels.push_back(voxel);
                }
            }
        }
        torsos.push_back(torso);
    }

    // Variation 5: Wide torso (broad shoulders)
    {
        BodyPartTemplate torso;
        torso.type = BodyPartType::Torso;
        torso.dimensions = {6, 6, 2};
        torso.attachmentPoint = {0.0f, 0.0f, 0.0f};

        for (int x = -3; x < 3; ++x) {
            for (int y = 0; y < 6; ++y) {
                for (int z = -1; z < 1; ++z) {
                    CharacterVoxel voxel;
                    voxel.position = {x, y, z};
                    voxel.color = {0.2f, 0.4f, 0.8f};
                    voxel.boneIndex = 1;
                    torso.voxels.push_back(voxel);
                }
            }
        }
        torsos.push_back(torso);
    }
}

void BodyPartLibrary::generateLimbVariations()
{
    // Generate arm variations
    std::vector<BodyPartTemplate>& leftArms = templates[BodyPartType::LeftArm];
    std::vector<BodyPartTemplate>& rightArms = templates[BodyPartType::RightArm];

    // Variation 1: Standard arms
    {
        BodyPartTemplate leftArm;
        leftArm.type = BodyPartType::LeftArm;
        leftArm.dimensions = {2, 4, 2};
        leftArm.attachmentPoint = {2.0f, 5.0f, 0.0f};

        for (int x = 0; x < 2; ++x) {
            for (int y = 0; y < 4; ++y) {
                for (int z = -1; z < 1; ++z) {
                    CharacterVoxel voxel;
                    voxel.position = {-x, -y, z};
                    voxel.color = {0.8f, 0.6f, 0.5f};
                    voxel.boneIndex = 5; // LeftArm bone
                    leftArm.voxels.push_back(voxel);
                }
            }
        }
        leftArms.push_back(leftArm);

        // Mirror for right arm
        BodyPartTemplate rightArm = leftArm;
        rightArm.type = BodyPartType::RightArm;
        rightArm.attachmentPoint = {-2.0f, 5.0f, 0.0f};
        for (auto& voxel : rightArm.voxels) {
            voxel.position.x = -voxel.position.x;
            voxel.boneIndex = 8; // RightArm bone
        }
        rightArms.push_back(rightArm);
    }

    // Variation 2: Thick arms (muscular)
    {
        BodyPartTemplate leftArm;
        leftArm.type = BodyPartType::LeftArm;
        leftArm.dimensions = {3, 4, 2};
        leftArm.attachmentPoint = {2.0f, 5.0f, 0.0f};

        for (int x = 0; x < 3; ++x) {
            for (int y = 0; y < 4; ++y) {
                for (int z = -1; z < 1; ++z) {
                    CharacterVoxel voxel;
                    voxel.position = {-x, -y, z};
                    voxel.color = {0.8f, 0.6f, 0.5f};
                    voxel.boneIndex = 5;
                    leftArm.voxels.push_back(voxel);
                }
            }
        }
        leftArms.push_back(leftArm);

        BodyPartTemplate rightArm = leftArm;
        rightArm.type = BodyPartType::RightArm;
        rightArm.attachmentPoint = {-2.0f, 5.0f, 0.0f};
        for (auto& voxel : rightArm.voxels) {
            voxel.position.x = -voxel.position.x;
            voxel.boneIndex = 8;
        }
        rightArms.push_back(rightArm);
    }

    // Variation 3: Thin arms (slim)
    {
        BodyPartTemplate leftArm;
        leftArm.type = BodyPartType::LeftArm;
        leftArm.dimensions = {1, 4, 1};
        leftArm.attachmentPoint = {2.0f, 5.0f, 0.0f};

        for (int y = 0; y < 4; ++y) {
            CharacterVoxel voxel;
            voxel.position = {0, -y, 0};
            voxel.color = {0.8f, 0.6f, 0.5f};
            voxel.boneIndex = 5;
            leftArm.voxels.push_back(voxel);
        }
        leftArms.push_back(leftArm);

        BodyPartTemplate rightArm = leftArm;
        rightArm.type = BodyPartType::RightArm;
        rightArm.attachmentPoint = {-2.0f, 5.0f, 0.0f};
        for (auto& voxel : rightArm.voxels) {
            voxel.boneIndex = 8;
        }
        rightArms.push_back(rightArm);
    }

    // Generate leg variations
    std::vector<BodyPartTemplate>& leftLegs = templates[BodyPartType::LeftLeg];
    std::vector<BodyPartTemplate>& rightLegs = templates[BodyPartType::RightLeg];

    // Variation 1: Standard legs
    {
        BodyPartTemplate leftLeg;
        leftLeg.type = BodyPartType::LeftLeg;
        leftLeg.dimensions = {2, 4, 2};
        leftLeg.attachmentPoint = {-1.0f, 0.0f, 0.0f};

        for (int x = 0; x < 2; ++x) {
            for (int y = 0; y < 4; ++y) {
                for (int z = -1; z < 1; ++z) {
                    CharacterVoxel voxel;
                    voxel.position = {-x, -y, z};
                    voxel.color = {0.3f, 0.3f, 0.3f}; // Pants
                    voxel.boneIndex = 11;             // LeftLeg bone
                    leftLeg.voxels.push_back(voxel);
                }
            }
        }
        leftLegs.push_back(leftLeg);

        BodyPartTemplate rightLeg = leftLeg;
        rightLeg.type = BodyPartType::RightLeg;
        rightLeg.attachmentPoint = {1.0f, 0.0f, 0.0f};
        for (auto& voxel : rightLeg.voxels) {
            voxel.position.x = -voxel.position.x;
            voxel.boneIndex = 14; // RightLeg bone
        }
        rightLegs.push_back(rightLeg);
    }

    // Variation 2: Thick legs
    {
        BodyPartTemplate leftLeg;
        leftLeg.type = BodyPartType::LeftLeg;
        leftLeg.dimensions = {3, 4, 2};
        leftLeg.attachmentPoint = {-1.0f, 0.0f, 0.0f};

        for (int x = 0; x < 3; ++x) {
            for (int y = 0; y < 4; ++y) {
                for (int z = -1; z < 1; ++z) {
                    CharacterVoxel voxel;
                    voxel.position = {-x, -y, z};
                    voxel.color = {0.3f, 0.3f, 0.3f};
                    voxel.boneIndex = 11;
                    leftLeg.voxels.push_back(voxel);
                }
            }
        }
        leftLegs.push_back(leftLeg);

        BodyPartTemplate rightLeg = leftLeg;
        rightLeg.type = BodyPartType::RightLeg;
        rightLeg.attachmentPoint = {1.0f, 0.0f, 0.0f};
        for (auto& voxel : rightLeg.voxels) {
            voxel.position.x = -voxel.position.x;
            voxel.boneIndex = 14;
        }
        rightLegs.push_back(rightLeg);
    }

    // Variation 3: Long legs
    {
        BodyPartTemplate leftLeg;
        leftLeg.type = BodyPartType::LeftLeg;
        leftLeg.dimensions = {2, 5, 2};
        leftLeg.attachmentPoint = {-1.0f, 0.0f, 0.0f};

        for (int x = 0; x < 2; ++x) {
            for (int y = 0; y < 5; ++y) {
                for (int z = -1; z < 1; ++z) {
                    CharacterVoxel voxel;
                    voxel.position = {-x, -y, z};
                    voxel.color = {0.3f, 0.3f, 0.3f};
                    voxel.boneIndex = 11;
                    leftLeg.voxels.push_back(voxel);
                }
            }
        }
        leftLegs.push_back(leftLeg);

        BodyPartTemplate rightLeg = leftLeg;
        rightLeg.type = BodyPartType::RightLeg;
        rightLeg.attachmentPoint = {1.0f, 0.0f, 0.0f};
        for (auto& voxel : rightLeg.voxels) {
            voxel.position.x = -voxel.position.x;
            voxel.boneIndex = 14;
        }
        rightLegs.push_back(rightLeg);
    }

    // Generate hand and foot placeholders (simple blocks)
    std::vector<BodyPartTemplate>& leftHands = templates[BodyPartType::LeftHand];
    std::vector<BodyPartTemplate>& rightHands = templates[BodyPartType::RightHand];
    std::vector<BodyPartTemplate>& leftFeet = templates[BodyPartType::LeftFoot];
    std::vector<BodyPartTemplate>& rightFeet = templates[BodyPartType::RightFoot];

    // Standard hands
    {
        BodyPartTemplate leftHand;
        leftHand.type = BodyPartType::LeftHand;
        leftHand.dimensions = {2, 2, 2};
        leftHand.attachmentPoint = {-2.0f, 0.0f, 0.0f};

        for (int x = 0; x < 2; ++x) {
            for (int y = 0; y < 2; ++y) {
                for (int z = -1; z < 1; ++z) {
                    CharacterVoxel voxel;
                    voxel.position = {-x, -y, z};
                    voxel.color = {0.8f, 0.6f, 0.5f};
                    voxel.boneIndex = 6; // LeftHand bone
                    leftHand.voxels.push_back(voxel);
                }
            }
        }
        leftHands.push_back(leftHand);

        BodyPartTemplate rightHand = leftHand;
        rightHand.type = BodyPartType::RightHand;
        rightHand.attachmentPoint = {2.0f, 0.0f, 0.0f};
        for (auto& voxel : rightHand.voxels) {
            voxel.position.x = -voxel.position.x;
            voxel.boneIndex = 9; // RightHand bone
        }
        rightHands.push_back(rightHand);
    }

    // Standard feet
    {
        BodyPartTemplate leftFoot;
        leftFoot.type = BodyPartType::LeftFoot;
        leftFoot.dimensions = {2, 1, 3};
        leftFoot.attachmentPoint = {-1.0f, -4.0f, 0.0f};

        for (int x = 0; x < 2; ++x) {
            for (int z = -1; z < 2; ++z) {
                CharacterVoxel voxel;
                voxel.position = {-x, 0, z};
                voxel.color = {0.2f, 0.2f, 0.2f}; // Shoes
                voxel.boneIndex = 12;             // LeftFoot bone
                leftFoot.voxels.push_back(voxel);
            }
        }
        leftFeet.push_back(leftFoot);

        BodyPartTemplate rightFoot = leftFoot;
        rightFoot.type = BodyPartType::RightFoot;
        rightFoot.attachmentPoint = {1.0f, -4.0f, 0.0f};
        for (auto& voxel : rightFoot.voxels) {
            voxel.position.x = -voxel.position.x;
            voxel.boneIndex = 15; // RightFoot bone
        }
        rightFeet.push_back(rightFoot);
    }
}

} // namespace fresh
