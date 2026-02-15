#include "rpg/TradingSystem.h"

namespace fresh
{
namespace rpg
{

TradingSystem::TradingSystem()
{
    initializeDefaultOffers();
}

void TradingSystem::addOffer(const TradeOffer& offer)
{
    offers[offer.resource] = offer;
}

void TradingSystem::removeOffer(ResourceType resource)
{
    offers.erase(resource);
}

const TradeOffer* TradingSystem::getOffer(ResourceType resource) const
{
    auto it = offers.find(resource);
    return (it != offers.end()) ? &it->second : nullptr;
}

std::vector<TradeOffer> TradingSystem::getAllOffers() const
{
    std::vector<TradeOffer> result;
    result.reserve(offers.size());
    for (const auto& pair : offers) {
        result.push_back(pair.second);
    }
    return result;
}

bool TradingSystem::buyResource(ResourceType resource, float quantity, Inventory& playerInventory,
                                float& playerCredits)
{
    const TradeOffer* offer = getOffer(resource);
    if (!offer || !offer->canBuy || quantity > offer->availableQuantity) {
        return false;
    }

    float cost = calculateBuyPrice(resource, quantity);
    if (playerCredits < cost) {
        return false;
    }

    if (!playerInventory.addResource(resource, quantity)) {
        return false;
    }

    playerCredits -= cost;
    return true;
}

bool TradingSystem::sellResource(ResourceType resource, float quantity, Inventory& playerInventory,
                                 float& playerCredits)
{
    const TradeOffer* offer = getOffer(resource);
    if (!offer || !offer->canSell) {
        return false;
    }

    if (!playerInventory.hasResource(resource, quantity)) {
        return false;
    }

    float earnings = calculateSellPrice(resource, quantity);

    if (!playerInventory.removeResource(resource, quantity)) {
        return false;
    }

    playerCredits += earnings;
    return true;
}

float TradingSystem::calculateBuyPrice(ResourceType resource, float quantity) const
{
    const TradeOffer* offer = getOffer(resource);
    return offer ? (offer->pricePerUnit * quantity) : 0.0f;
}

float TradingSystem::calculateSellPrice(ResourceType resource, float quantity) const
{
    const TradeOffer* offer = getOffer(resource);
    // Sell price is typically lower than buy price
    return offer ? (offer->pricePerUnit * quantity * 0.7f) : 0.0f;
}

void TradingSystem::updatePrices(float deltaTime)
{
    (void)deltaTime; // Unused - placeholder for future implementation
    // Placeholder for dynamic pricing based on supply/demand
    // Could implement price fluctuations over time
}

void TradingSystem::initializeDefaultOffers()
{
    // Iron
    addOffer({ResourceType::Iron, 10.0f, 1000.0f, true, true});

    // Titanium
    addOffer({ResourceType::Titanium, 25.0f, 500.0f, true, true});

    // Naonite
    addOffer({ResourceType::Naonite, 50.0f, 300.0f, true, true});

    // Trinium
    addOffer({ResourceType::Trinium, 100.0f, 200.0f, true, true});

    // Xanion
    addOffer({ResourceType::Xanion, 200.0f, 100.0f, true, true});

    // Ogonite
    addOffer({ResourceType::Ogonite, 400.0f, 50.0f, true, true});

    // Avorion
    addOffer({ResourceType::Avorion, 800.0f, 25.0f, true, true});

    // Energy (can only buy)
    addOffer({ResourceType::Energy, 5.0f, 10000.0f, true, false});
}

} // namespace rpg
} // namespace fresh
