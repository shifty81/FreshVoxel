-- Example Lua script for dialogue system
-- This script can be called from dialogue nodes

-- Quest management example
function startQuest(questId)
    print("Starting quest: " .. questId)
    -- Add quest to player's quest log
    -- Update game state
    return true
end

function completeQuest(questId)
    print("Completing quest: " .. questId)
    -- Mark quest as complete
    -- Give rewards
    return true
end

-- Condition checking example
function hasItem(itemId)
    -- Check if player has specific item
    print("Checking for item: " .. itemId)
    -- return player.inventory:hasItem(itemId)
    return false -- placeholder
end

function hasCompletedQuest(questId)
    -- Check if quest is completed
    print("Checking quest status: " .. questId)
    -- return questManager:isCompleted(questId)
    return false -- placeholder
end

-- Game state management
function setGameFlag(flagName, value)
    print("Setting game flag: " .. flagName .. " = " .. tostring(value))
    -- Store flag in game state
end

function getGameFlag(flagName)
    print("Getting game flag: " .. flagName)
    -- Retrieve flag from game state
    return false -- placeholder
end

-- Event triggers
function triggerCutscene(cutsceneId)
    print("Triggering cutscene: " .. cutsceneId)
    -- Play cutscene
end

function spawnEnemy(enemyType, x, y, z)
    print("Spawning enemy: " .. enemyType .. " at (" .. x .. ", " .. y .. ", " .. z .. ")")
    -- Spawn enemy at location
end

function giveReward(rewardType, amount)
    print("Giving reward: " .. rewardType .. " x" .. amount)
    -- Give reward to player
end

-- Example complex dialogue logic
function processDialogueChoice(choiceId)
    if choiceId == "accept_quest" then
        startQuest("ancient_artifact_quest")
        giveReward("gold", 10)
        setGameFlag("talked_to_elder", true)
        return "quest_accepted"
    elseif choiceId == "decline_quest" then
        setGameFlag("declined_elder_quest", true)
        return "quest_declined"
    else
        return "continue"
    end
end

print("Dialogue Lua script loaded successfully!")
