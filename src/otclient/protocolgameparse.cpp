/*
 * Copyright (c) 2010-2012 OTClient <https://github.com/edubart/otclient>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "protocolgame.h"

#include "localplayer.h"
#include "thingtypemanager.h"
#include "game.h"
#include "map.h"
#include "item.h"
#include "effect.h"
#include "missile.h"
#include "tile.h"
#include "luavaluecasts.h"
#include <framework/core/eventdispatcher.h>

void ProtocolGame::parseMessage(const InputMessagePtr& msg)
{
    int opcode = -1;
    int prevOpcode = -1;

    try {
        while(!msg->eof()) {
            opcode = msg->getU8();

            if(!m_gameInitialized && opcode >= Proto::GameServerFirstGameOpcode) {
                g_game.processGameStart();
                m_gameInitialized = true;
            }

            // try to parse in lua first
            int readPos = msg->getReadPos();
            if(callLuaField<bool>("onOpcode", opcode, msg))
                continue;
            else
                msg->setReadPos(readPos); // restore read pos

            switch(opcode) {
            case Proto::GameServerInitGame:
                parseInitGame(msg);
                break;
            case Proto::GameServerGMActions:
                parseGMActions(msg);
                break;
            case Proto::GameServerLoginError:
                parseLoginError(msg);
                break;
            case Proto::GameServerLoginAdvice:
                parseLoginAdvice(msg);
                break;
            case Proto::GameServerLoginWait:
                parseLoginWait(msg);
                break;
            case Proto::GameServerPing:
            case Proto::GameServerPingBack:
                if((opcode == Proto::GameServerPing && g_game.getClientVersion() >= 953) ||
                   (opcode == Proto::GameServerPingBack && g_game.getClientVersion() < 953))
                    parsePingBack(msg);
                else
                    parsePing(msg);
                break;
            case Proto::GameServerChallange:
                parseChallange(msg);
                break;
            case Proto::GameServerDeath:
                parseDeath(msg);
                break;
            case Proto::GameServerFullMap:
                parseMapDescription(msg);
                break;
            case Proto::GameServerMapTopRow:
                parseMapMoveNorth(msg);
                break;
            case Proto::GameServerMapRightRow:
                parseMapMoveEast(msg);
                break;
            case Proto::GameServerMapBottomRow:
                parseMapMoveSouth(msg);
                break;
            case Proto::GameServerMapLeftRow:
                parseMapMoveWest(msg);
                break;
            case Proto::GameServerUpdateTile:
                parseUpdateTile(msg);
                break;
            case Proto::GameServerCreateOnMap:
                parseTileAddThing(msg);
                break;
            case Proto::GameServerChangeOnMap:
                parseTileTransformThing(msg);
                break;
            case Proto::GameServerDeleteOnMap:
                parseTileRemoveThing(msg);
                break;
            case Proto::GameServerMoveCreature:
                parseCreatureMove(msg);
                break;
            case Proto::GameServerOpenContainer:
                parseOpenContainer(msg);
                break;
            case Proto::GameServerCloseContainer:
                parseCloseContainer(msg);
                break;
            case Proto::GameServerCreateContainer:
                parseContainerAddItem(msg);
                break;
            case Proto::GameServerChangeInContainer:
                parseContainerUpdateItem(msg);
                break;
            case Proto::GameServerDeleteInContainer:
                parseContainerRemoveItem(msg);
                break;
            case Proto::GameServerSetInventory:
                parseAddInventoryItem(msg);
                break;
            case Proto::GameServerDeleteInventory:
                parseRemoveInventoryItem(msg);
                break;
            case Proto::GameServerOpenNpcTrade:
                parseOpenNpcTrade(msg);
                break;
            case Proto::GameServerPlayerGoods:
                parsePlayerGoods(msg);
                break;
            case Proto::GameServerCloseNpcTrade:
                parseCloseNpcTrade(msg);
                break;
            case Proto::GameServerOwnTrade:
                parseOwnTrade(msg);
                break;
            case Proto::GameServerCounterTrade:
                parseCounterTrade(msg);
                break;
            case Proto::GameServerCloseTrade:
                parseCloseTrade(msg);
                break;
            case Proto::GameServerAmbient:
                parseWorldLight(msg);
                break;
            case Proto::GameServerGraphicalEffect:
                parseMagicEffect(msg);
                break;
            case Proto::GameServerTextEffect:
                parseAnimatedText(msg);
                break;
            case Proto::GameServerMissleEffect:
                parseDistanceMissile(msg);
                break;
            case Proto::GameServerMarkCreature:
                parseCreatureMark(msg);
                break;
            case Proto::GameServerTrappers:
                parseTrappers(msg);
                break;
            case Proto::GameServerCreatureHealth:
                parseCreatureHealth(msg);
                break;
            case Proto::GameServerCreatureLight:
                parseCreatureLight(msg);
                break;
            case Proto::GameServerCreatureOutfit:
                parseCreatureOutfit(msg);
                break;
            case Proto::GameServerCreatureSpeed:
                parseCreatureSpeed(msg);
                break;
            case Proto::GameServerCreatureSkull:
                parseCreatureSkulls(msg);
                break;
            case Proto::GameServerCreatureParty:
                parseCreatureShields(msg);
                break;
            case Proto::GameServerCreatureUnpass:
                parseCreatureUnpass(msg);
                break;
            case Proto::GameServerEditText:
                parseEditText(msg);
                break;
            case Proto::GameServerEditList:
                parseEditList(msg);
                break;
            case Proto::GameServerPlayerData:
                parsePlayerStats(msg);
                break;
            case Proto::GameServerPlayerSkills:
                parsePlayerSkills(msg);
                break;
            case Proto::GameServerPlayerState:
                parsePlayerState(msg);
                break;
            case Proto::GameServerClearTarget:
                parsePlayerCancelAttack(msg);
                break;
            case Proto::GameServerTalk:
                parseTalk(msg);
                break;
            case Proto::GameServerChannels:
                parseChannelList(msg);
                break;
            case Proto::GameServerOpenChannel:
                parseOpenChannel(msg);
                break;
            case Proto::GameServerOpenPrivateChannel:
                parseOpenPrivateChannel(msg);
                break;
            case Proto::GameServerRuleViolationChannel:
                parseRuleViolationChannel(msg);
                break;
            case Proto::GameServerRuleViolationRemove:
                parseRuleViolationRemove(msg);
                break;
            case Proto::GameServerRuleViolationCancel:
                parseRuleViolationCancel(msg);
                break;
            case Proto::GameServerRuleViolationLock:
                parseRuleViolationLock(msg);
                break;
            case Proto::GameServerOpenOwnChannel:
                parseOpenOwnPrivateChannel(msg);
                break;
            case Proto::GameServerCloseChannel:
                parseCloseChannel(msg);
                break;
            case Proto::GameServerTextMessage:
                parseTextMessage(msg);
                break;
            case Proto::GameServerCancelWalk:
                parseCancelWalk(msg);
                break;
            case Proto::GameServerWalkWait:
                parseWalkWait(msg);
                break;
            case Proto::GameServerFloorChangeUp:
                parseFloorChangeUp(msg);
                break;
            case Proto::GameServerFloorChangeDown:
                parseFloorChangeDown(msg);
                break;
            case Proto::GameServerChooseOutfit:
                parseOpenOutfitWindow(msg);
                break;
            case Proto::GameServerVipAdd:
                parseVipAdd(msg);
                break;
            case Proto::GameServerVipLogin:
                parseVipLogin(msg);
                break;
            case Proto::GameServerVipLogout:
                parseVipLogout(msg);
                break;
            case Proto::GameServerTutorialHint:
                parseTutorialHint(msg);
                break;
            case Proto::GameServerAutomapFlag:
                parseAutomapFlag(msg);
                break;
            case Proto::GameServerQuestLog:
                parseQuestLog(msg);
                break;
            case Proto::GameServerQuestLine:
                parseQuestLine(msg);
                break;
            // PROTOCOL>=870
            case Proto::GameServerSpellDelay:
                parseSpellCooldown(msg);
                break;
            case Proto::GameServerSpellGroupDelay:
                parseSpellGroupCooldown(msg);
                break;
            case Proto::GameServerMultiUseDelay:
                parseMultiUseCooldown(msg);
                break;
            // PROTOCOL>=910
            case Proto::GameServerChannelEvent:
                parseChannelEvent(msg);
                break;
            case Proto::GameServerItemInfo:
                parseItemInfo(msg);
                break;
            case Proto::GameServerPlayerInventory:
                parsePlayerInventory(msg);
                break;
            // PROTOCOL>=950
            case Proto::GameServerPlayerDataBasic:
                parsePlayerInfo(msg);
                break;
            // otclient ONLY
            case Proto::GameServerExtendedOpcode:
                parseExtendedOpcode(msg);
                break;
            default:
                stdext::throw_exception(stdext::format("unhandled opcode %d", (int)opcode));
                break;
            }
            prevOpcode = opcode;
        }
    } catch(stdext::exception& e) {
        g_logger.error(stdext::format("ProtocolGame parse message exception (%d bytes unread, last opcode is %d, prev opcode is %d): %s",
                                      msg->getUnreadSize(), opcode, prevOpcode, e.what()));
    }
}

void ProtocolGame::parseInitGame(const InputMessagePtr& msg)
{
    uint playerId = msg->getU32();
    int serverBeat = msg->getU16();
    bool canReportBugs = msg->getU8();

    m_localPlayer->setId(playerId);
    g_game.setServerBeat(serverBeat);
    g_game.setCanReportBugs(canReportBugs);
}

void ProtocolGame::parseGMActions(const InputMessagePtr& msg)
{
    std::vector<uint8> actions;

    int numViolationReasons;

    if(g_game.getClientVersion() >= 860)
        numViolationReasons = 20;
    else if(g_game.getClientVersion() >= 854)
        numViolationReasons = 19;
    else
        numViolationReasons = 32;

    for(int i = 0; i < numViolationReasons; ++i)
        actions.push_back(msg->getU8());
    g_game.processGMActions(actions);
}

void ProtocolGame::parseLoginError(const InputMessagePtr& msg)
{
    std::string error = msg->getString();

    g_game.processLoginError(error);
}

void ProtocolGame::parseLoginAdvice(const InputMessagePtr& msg)
{
    std::string message = msg->getString();

    g_game.processLoginAdvice(message);
}

void ProtocolGame::parseLoginWait(const InputMessagePtr& msg)
{
    std::string message = msg->getString();
    int time = msg->getU8();

    g_game.processLoginWait(message, time);
}

void ProtocolGame::parsePing(const InputMessagePtr& msg)
{
    g_game.processPing();
    sendPingBack();
}

void ProtocolGame::parsePingBack(const InputMessagePtr& msg)
{
    g_game.processPingBack(m_pingTimer.ticksElapsed());
}

void ProtocolGame::parseChallange(const InputMessagePtr& msg)
{
    uint timestamp = msg->getU32();
    uint8 random = msg->getU8();
    sendLoginPacket(timestamp, random);
}

void ProtocolGame::parseDeath(const InputMessagePtr& msg)
{
    int penality = 100;
    if(g_game.getFeature(Otc::GamePenalityOnDeath))
        penality = msg->getU8();
    g_game.processDeath(penality);
}

void ProtocolGame::parseMapDescription(const InputMessagePtr& msg)
{
    Position pos = getPosition(msg);
    g_map.setCentralPosition(pos);
    setMapDescription(msg, pos.x - Otc::AWARE_X_LEFT_TILES, pos.y - Otc::AWARE_Y_TOP_TILES, pos.z, Otc::AWARE_X_TILES, Otc::AWARE_Y_TILES);
}

void ProtocolGame::parseMapMoveNorth(const InputMessagePtr& msg)
{
    Position pos = g_map.getCentralPosition();
    pos.y--;

    setMapDescription(msg, pos.x - Otc::AWARE_X_LEFT_TILES, pos.y - Otc::AWARE_Y_TOP_TILES, pos.z, Otc::AWARE_X_TILES, 1);
    g_map.setCentralPosition(pos);
}

void ProtocolGame::parseMapMoveEast(const InputMessagePtr& msg)
{
    Position pos = g_map.getCentralPosition();
    pos.x++;

    setMapDescription(msg, pos.x + Otc::AWARE_X_RIGHT_TILES, pos.y - Otc::AWARE_Y_TOP_TILES, pos.z, 1, Otc::AWARE_Y_TILES);
    g_map.setCentralPosition(pos);
}

void ProtocolGame::parseMapMoveSouth(const InputMessagePtr& msg)
{
    Position pos = g_map.getCentralPosition();
    pos.y++;

    setMapDescription(msg, pos.x - Otc::AWARE_X_LEFT_TILES, pos.y + Otc::AWARE_Y_BOTTOM_TILES, pos.z, Otc::AWARE_X_TILES, 1);
    g_map.setCentralPosition(pos);
}

void ProtocolGame::parseMapMoveWest(const InputMessagePtr& msg)
{
    Position pos = g_map.getCentralPosition();
    pos.x--;

    setMapDescription(msg, pos.x - Otc::AWARE_X_LEFT_TILES, pos.y - Otc::AWARE_Y_TOP_TILES, pos.z, 1, Otc::AWARE_Y_TILES);
    g_map.setCentralPosition(pos);
}

void ProtocolGame::parseUpdateTile(const InputMessagePtr& msg)
{
    Position tilePos = getPosition(msg);
    setTileDescription(msg, tilePos);
}

void ProtocolGame::parseTileAddThing(const InputMessagePtr& msg)
{
    Position pos = getPosition(msg);
    int stackPos = -1;

    if(g_game.getClientVersion() >= 854)
        stackPos = msg->getU8();

    ThingPtr thing = getThing(msg);
    g_map.addThing(thing, pos, stackPos);
}

void ProtocolGame::parseTileTransformThing(const InputMessagePtr& msg)
{
    ThingPtr thing = getMappedThing(msg);
    ThingPtr newThing = getThing(msg);

    if(!thing) {
        g_logger.traceError("no thing");
        return;
    }

    Position pos = thing->getPosition();
    int stackpos = thing->getStackpos();

    if(!g_map.removeThing(thing)) {
        g_logger.traceError("unable to remove thing");
        return;
    }

    g_map.addThing(newThing, pos, stackpos);
}

void ProtocolGame::parseTileRemoveThing(const InputMessagePtr& msg)
{
    ThingPtr thing = getMappedThing(msg);
    if(!thing) {
        g_logger.traceError("no thing");
        return;
    }

    g_map.removeThing(thing);
}

void ProtocolGame::parseCreatureMove(const InputMessagePtr& msg)
{
    ThingPtr thing = getMappedThing(msg);
    Position newPos = getPosition(msg);

    if(!thing || !thing->isCreature()) {
        g_logger.traceError("no creature found to move");
        return;
    }

    int stackPos = -2;
    // newer protocols stores creatures in reverse order
    if(!g_game.getClientVersion()  >= 854)
        stackPos = -1;

    g_map.removeThing(thing);
    g_map.addThing(thing, newPos, stackPos);
}

void ProtocolGame::parseOpenContainer(const InputMessagePtr& msg)
{
    int containerId = msg->getU8();
    ItemPtr containerItem = getItem(msg);
    std::string name = msg->getString();
    int capacity = msg->getU8();
    bool hasParent = (msg->getU8() != 0);
    int itemCount = msg->getU8();

    std::vector<ItemPtr> items(itemCount);
    for(int i = 0; i < itemCount; i++)
        items[i] = getItem(msg);

    g_game.processOpenContainer(containerId, containerItem, name, capacity, hasParent, items);
}

void ProtocolGame::parseCloseContainer(const InputMessagePtr& msg)
{
    int containerId = msg->getU8();
    g_game.processCloseContainer(containerId);
}

void ProtocolGame::parseContainerAddItem(const InputMessagePtr& msg)
{
    int containerId = msg->getU8();
    ItemPtr item = getItem(msg);
    g_game.processContainerAddItem(containerId, item);
}

void ProtocolGame::parseContainerUpdateItem(const InputMessagePtr& msg)
{
    int containerId = msg->getU8();
    int slot = msg->getU8();
    ItemPtr item = getItem(msg);
    g_game.processContainerUpdateItem(containerId, slot, item);
}

void ProtocolGame::parseContainerRemoveItem(const InputMessagePtr& msg)
{
    int containerId = msg->getU8();
    int slot = msg->getU8();
    g_game.processContainerRemoveItem(containerId, slot);
}

void ProtocolGame::parseAddInventoryItem(const InputMessagePtr& msg)
{
    int slot = msg->getU8();
    ItemPtr item = getItem(msg);
    g_game.processInventoryChange(slot, item);
}

void ProtocolGame::parseRemoveInventoryItem(const InputMessagePtr& msg)
{
    int slot = msg->getU8();
    g_game.processInventoryChange(slot, ItemPtr());
}

void ProtocolGame::parseOpenNpcTrade(const InputMessagePtr& msg)
{
    std::vector<std::tuple<ItemPtr, std::string, int, int, int>> items;
    std::string npcName;

    if(g_game.getFeature(Otc::GameNameOnNpcTrade))
        npcName = msg->getString();

    int listCount;

    if(g_game.getClientVersion() >= 960)
        listCount = msg->getU16();
    else
        listCount = msg->getU8();

    for(int i = 0; i < listCount; ++i) {
        uint16 itemId = msg->getU16();
        uint8 count = msg->getU8();

        ItemPtr item = Item::create(itemId);
        item->setCountOrSubType(count);

        std::string name = msg->getString();
        int weight = msg->getU32();
        int buyPrice = msg->getU32();
        int sellPrice = msg->getU32();
        items.push_back(std::make_tuple(item, name, weight, buyPrice, sellPrice));
    }

    g_game.processOpenNpcTrade(items);
}

void ProtocolGame::parsePlayerGoods(const InputMessagePtr& msg)
{
    std::vector<std::tuple<ItemPtr, int>> goods;

    int money = msg->getU32();
    int size = msg->getU8();
    for(int i = 0; i < size; i++) {
        int itemId = msg->getU16();
        int count = msg->getU8();

        goods.push_back(std::make_tuple(Item::create(itemId), count));
    }

    g_game.processPlayerGoods(money, goods);
}

void ProtocolGame::parseCloseNpcTrade(const InputMessagePtr&)
{
    g_game.processCloseNpcTrade();
}

void ProtocolGame::parseOwnTrade(const InputMessagePtr& msg)
{
    std::string name = msg->getString();
    int count = msg->getU8();

    std::vector<ItemPtr> items(count);
    for(int i = 0; i < count; i++)
        items[i] = getItem(msg);

    g_game.processOwnTrade(name, items);
}

void ProtocolGame::parseCounterTrade(const InputMessagePtr& msg)
{
    std::string name = msg->getString();
    int count = msg->getU8();

    std::vector<ItemPtr> items(count);
    for(int i = 0; i < count; i++)
        items[i] = getItem(msg);

    g_game.processCounterTrade(name, items);
}

void ProtocolGame::parseCloseTrade(const InputMessagePtr&)
{
    g_game.processCloseTrade();
}

void ProtocolGame::parseWorldLight(const InputMessagePtr& msg)
{
    Light light;
    light.intensity = msg->getU8();
    light.color = msg->getU8();

    g_map.setLight(light);
}

void ProtocolGame::parseMagicEffect(const InputMessagePtr& msg)
{
    Position pos = getPosition(msg);
    int effectId;
    if(g_game.getFeature(Otc::GameMagicEffectU16))
        effectId = msg->getU16();
    else
        effectId = msg->getU8();

    EffectPtr effect = EffectPtr(new Effect());
    effect->setId(effectId);
    g_map.addThing(effect, pos);
}

void ProtocolGame::parseAnimatedText(const InputMessagePtr& msg)
{
    Position position = getPosition(msg);
    int color = msg->getU8();
    std::string text = msg->getString();

    AnimatedTextPtr animatedText = AnimatedTextPtr(new AnimatedText);
    animatedText->setColor(color);
    animatedText->setText(text);
    g_map.addThing(animatedText, position);
}

void ProtocolGame::parseDistanceMissile(const InputMessagePtr& msg)
{
    Position fromPos = getPosition(msg);
    Position toPos = getPosition(msg);
    int shotId = msg->getU8();

    MissilePtr missile = MissilePtr(new Missile());
    missile->setId(shotId);
    missile->setPath(fromPos, toPos);
    g_map.addThing(missile, fromPos);
}

void ProtocolGame::parseCreatureMark(const InputMessagePtr& msg)
{
    uint id = msg->getU32();
    int color = msg->getU8();

    CreaturePtr creature = g_map.getCreatureById(id);
    if(creature)
        creature->addTimedSquare(color);
    else
        g_logger.traceError("could not get creature");
}

void ProtocolGame::parseTrappers(const InputMessagePtr& msg)
{
    int numTrappers = msg->getU8();

    if(numTrappers > 8)
        g_logger.traceError("too many trappers");

    for(int i=0;i<numTrappers;++i) {
        uint id = msg->getU32();
        CreaturePtr creature = g_map.getCreatureById(id);
        if(creature) {
            //TODO: set creature as trapper
        } else
            g_logger.traceError("could not get creature");
    }
}

void ProtocolGame::parseCreatureHealth(const InputMessagePtr& msg)
{
    uint id = msg->getU32();
    int healthPercent = msg->getU8();

    CreaturePtr creature = g_map.getCreatureById(id);
    if(creature)
        creature->setHealthPercent(healthPercent);

    // some servers has a bug in get spectators and sends unknown creatures updates
    // so this code is disabled
    /*
    else
        g_logger.traceError("could not get creature");
    */
}

void ProtocolGame::parseCreatureLight(const InputMessagePtr& msg)
{
    uint id = msg->getU32();

    Light light;
    light.intensity = msg->getU8();
    light.color = msg->getU8();

    CreaturePtr creature = g_map.getCreatureById(id);
    if(creature)
        creature->setLight(light);
    else
        g_logger.traceError("could not get creature");
}

void ProtocolGame::parseCreatureOutfit(const InputMessagePtr& msg)
{
    uint id = msg->getU32();
    Outfit outfit = getOutfit(msg);

    CreaturePtr creature = g_map.getCreatureById(id);
    if(creature)
        creature->setOutfit(outfit);
    else
        g_logger.traceError("could not get creature");
}

void ProtocolGame::parseCreatureSpeed(const InputMessagePtr& msg)
{
    uint id = msg->getU32();
    int speed = msg->getU16();

    CreaturePtr creature = g_map.getCreatureById(id);
    if(creature)
        creature->setSpeed(speed);

    // some servers has a bug in get spectators and sends unknown creatures updates
    // so this code is disabled
    /*
    else
        g_logger.traceError("could not get creature");
    */
}

void ProtocolGame::parseCreatureSkulls(const InputMessagePtr& msg)
{
    uint id = msg->getU32();
    int skull = msg->getU8();

    CreaturePtr creature = g_map.getCreatureById(id);
    if(creature)
        creature->setSkull(skull);
    else
        g_logger.traceError("could not get creature");
}

void ProtocolGame::parseCreatureShields(const InputMessagePtr& msg)
{
    uint id = msg->getU32();
    int shield = msg->getU8();

    CreaturePtr creature = g_map.getCreatureById(id);
    if(creature)
        creature->setShield(shield);
    else
        g_logger.traceError("could not get creature");
}

void ProtocolGame::parseCreatureUnpass(const InputMessagePtr& msg)
{
    uint id = msg->getU32();
    bool unpass = msg->getU8();

    CreaturePtr creature = g_map.getCreatureById(id);
    if(creature)
        creature->setPassable(!unpass);
    else
        g_logger.traceError("could not get creature");
}

void ProtocolGame::parseEditText(const InputMessagePtr& msg)
{
    uint id = msg->getU32();
    int itemId = msg->getU16();
    int maxLength = msg->getU16();
    std::string text = msg->getString();
    std::string writter = msg->getString();
    std::string date = msg->getString();

    g_game.processEditText(id, itemId, maxLength, text, writter, date);
}

void ProtocolGame::parseEditList(const InputMessagePtr& msg)
{
    int doorId = msg->getU8();
    uint id = msg->getU32();
    const std::string& text = msg->getString();

    g_game.processEditList(id, doorId, text);
}

void ProtocolGame::parsePlayerInfo(const InputMessagePtr& msg)
{
    bool premium = msg->getU8(); // premium
    int vocation = msg->getU8(); // vocation
    int spellCount = msg->getU16();
    for(int i=0;i<spellCount;++i) {
        int spellId = msg->getU8(); // spell id - TODO: add to local player
    }

    m_localPlayer->setPremium(premium);
    m_localPlayer->setVocation(vocation);
}

void ProtocolGame::parsePlayerStats(const InputMessagePtr& msg)
{
    double health = msg->getU16();
    double maxHealth = msg->getU16();

    double freeCapacity;
    if(g_game.getFeature(Otc::GameDoubleFreeCapacity))
        freeCapacity = msg->getU32() / 100.0;
    else
        freeCapacity = msg->getU16() / 100.0;

    double totalCapacity;
    if(g_game.getFeature(Otc::GameTotalCapacity))
        totalCapacity = msg->getU32() / 100.0;

    double experience;
    if(g_game.getFeature(Otc::GameDoubleExperience))
        experience = msg->getU64();
    else
        experience = msg->getU32();

    double level = msg->getU16();
    double levelPercent = msg->getU8();
    double mana = msg->getU16();
    double maxMana = msg->getU16();
    double magicLevel = msg->getU8();

    if(g_game.getFeature(Otc::GameSkillsBase))
        msg->getU8(); // base magic level

    double magicLevelPercent = msg->getU8();
    double soul = msg->getU8();
    double stamina = msg->getU16();

    m_localPlayer->setHealth(health, maxHealth);
    m_localPlayer->setFreeCapacity(freeCapacity);
    m_localPlayer->setExperience(experience);
    m_localPlayer->setLevel(level, levelPercent);
    m_localPlayer->setMana(mana, maxMana);
    m_localPlayer->setMagicLevel(magicLevel, magicLevelPercent);
    m_localPlayer->setStamina(stamina);
    m_localPlayer->setSoul(soul);

    if(g_game.getFeature(Otc::GameSkillsBase))
        msg->getU16(); // base speed

    if(g_game.getFeature(Otc::GamePlayerRegenerationTime))
        msg->getU16();

    if(g_game.getFeature(Otc::GameOfflineTrainingTime))
        msg->getU16();
}

void ProtocolGame::parsePlayerSkills(const InputMessagePtr& msg)
{
    for(int skill = 0; skill < Otc::LastSkill; skill++) {
        int level = msg->getU8();
        if(g_game.getFeature(Otc::GameSkillsBase))
            msg->getU8(); // base

        int levelPercent = msg->getU8();

        m_localPlayer->setSkill((Otc::Skill)skill, level, levelPercent);
    }
}

void ProtocolGame::parsePlayerState(const InputMessagePtr& msg)
{
    int states = msg->getU16();
    m_localPlayer->setStates(states);
}

void ProtocolGame::parsePlayerCancelAttack(const InputMessagePtr& msg)
{
    uint seq = 0;
    if(g_game.getClientVersion() >= 860)
        seq = msg->getU32();

    g_game.processAttackCancel(seq);
}

void ProtocolGame::parseSpellCooldown(const InputMessagePtr& msg)
{
    msg->getU8(); // spell id
    msg->getU32(); // cooldown
}

void ProtocolGame::parseSpellGroupCooldown(const InputMessagePtr& msg)
{
    msg->getU8(); // group id
    msg->getU32(); // cooldown
}

void ProtocolGame::parseMultiUseCooldown(const InputMessagePtr& msg)
{
    msg->getU32(); // cooldown
}

void ProtocolGame::parseTalk(const InputMessagePtr& msg)
{
    msg->getU32(); // channel statement guid

    std::string name = msg->getString();
    int level = msg->getU16();
    Otc::MessageMode mode = Proto::translateMessageModeFromServer(msg->getU8());
    int channelId = 0;
    Position pos;

    switch(mode) {
        case Otc::MessageSay:
        case Otc::MessageWhisper:
        case Otc::MessageYell:
        case Otc::MessageMonsterSay:
        case Otc::MessageMonsterYell:
        case Otc::MessageNpcTo:
        case Otc::MessageNpcFrom:
        case Otc::MessageBarkLow:
        case Otc::MessageBarkLoud:
        case Otc::MessageSpell:
            pos = getPosition(msg);
            break;
        case Otc::MessageChannel:
        case Otc::MessageChannelManagement:
        case Otc::MessageChannelHighlight:
        case Otc::MessageGamemasterChannel:
            channelId = msg->getU16();
            break;
        case Otc::MessagePrivateFrom:
        case Otc::MessageGamemasterBroadcast:
        case Otc::MessageGamemasterPrivateFrom:
            break;
        case Otc::MessageRVRChannel:
            msg->getU32();
            break;
        default:
            stdext::throw_exception(stdext::format("unknown message mode %d", mode));
            break;
    }

    std::string text = msg->getString();

    g_game.processTalk(name, level, mode, text, channelId, pos);
}

void ProtocolGame::parseChannelList(const InputMessagePtr& msg)
{
    int count = msg->getU8();
    std::vector<std::tuple<int, std::string> > channelList;
    for(int i = 0; i < count; i++) {
        int id = msg->getU16();
        std::string name = msg->getString();
        channelList.push_back(std::make_tuple(id, name));
    }

    g_game.processChannelList(channelList);
}

void ProtocolGame::parseOpenChannel(const InputMessagePtr& msg)
{
    int channelId = msg->getU16();
    std::string name = msg->getString();

    if(g_game.getFeature(Otc::GameChannelPlayerList)) {
        int joinedPlayers = msg->getU16();
        for(int i=0;i<joinedPlayers;++i)
            msg->getString(); // player name
        int invitedPlayers = msg->getU16();
        for(int i=0;i<invitedPlayers;++i)
            msg->getString(); // player name
    }

    g_game.processOpenChannel(channelId, name);
}

void ProtocolGame::parseOpenPrivateChannel(const InputMessagePtr& msg)
{
    std::string name = msg->getString();

    g_game.processOpenPrivateChannel(name);
}

void ProtocolGame::parseOpenOwnPrivateChannel(const InputMessagePtr& msg)
{
    int channelId = msg->getU16();
    std::string name = msg->getString();

    g_game.processOpenOwnPrivateChannel(channelId, name);
}

void ProtocolGame::parseCloseChannel(const InputMessagePtr& msg)
{
    int channelId = msg->getU16();

    g_game.processCloseChannel(channelId);
}

void ProtocolGame::parseRuleViolationChannel(const InputMessagePtr& msg)
{
    int channelId = msg->getU16();

    g_game.processRuleViolationChannel(channelId);
}

void ProtocolGame::parseRuleViolationRemove(const InputMessagePtr& msg)
{
    std::string name = msg->getString();

    g_game.processRuleViolationRemove(name);
}

void ProtocolGame::parseRuleViolationCancel(const InputMessagePtr& msg)
{
    std::string name = msg->getString();

    g_game.processRuleViolationCancel(name);
}

void ProtocolGame::parseRuleViolationLock(const InputMessagePtr& msg)
{
    g_game.processRuleViolationLock();
}

void ProtocolGame::parseTextMessage(const InputMessagePtr& msg)
{
    int code = msg->getU8();
    Otc::MessageMode mode = Proto::translateMessageModeFromServer(code);
    std::string text;

    switch(mode) {
        case Otc::MessageChannelManagement: {
            int channel = msg->getU16();
            text = msg->getString();
            break;
        }
        case Otc::MessageDamageDealed:
        case Otc::MessageDamageReceived:
        case Otc::MessageDamageOthers: {
            Position pos = getPosition(msg);
            uint value[2];
            int color[2];

            // physical damage
            value[0] = msg->getU32();
            color[0] =  msg->getU8();

            // magic damage
            value[1] = msg->getU32();
            color[1] = msg->getU8();
            text = msg->getString();

            for(int i=0;i<2;++i) {
                if(value[i] == 0)
                    continue;
                AnimatedTextPtr animatedText = AnimatedTextPtr(new AnimatedText);
                animatedText->setColor(color[i]);
                animatedText->setText(stdext::to_string(value[i]));
                g_map.addThing(animatedText, pos);
            }
            break;
        }
        case Otc::MessageHeal:
        case Otc::MessageExp:
        case Otc::MessageHealOthers:
        case Otc::MessageExpOthers: {
            Position pos = getPosition(msg);
            uint value = msg->getU32();
            int color =  msg->getU8();
            text = msg->getString();

            AnimatedTextPtr animatedText = AnimatedTextPtr(new AnimatedText);
            animatedText->setColor(color);
            animatedText->setText(stdext::to_string(value));
            g_map.addThing(animatedText, pos);
            break;
        }
        case Otc::MessageInvalid:
            stdext::throw_exception(stdext::format("unknown message mode %d", mode));
            break;
        default:
            text = msg->getString();
            break;
    }

    g_game.processTextMessage(mode, text);
}

void ProtocolGame::parseCancelWalk(const InputMessagePtr& msg)
{
    Otc::Direction direction = (Otc::Direction)msg->getU8();

    g_game.processWalkCancel(direction);
}

void ProtocolGame::parseWalkWait(const InputMessagePtr& msg)
{
    int millis = msg->getU16();
    m_localPlayer->lockWalk(millis);
}

void ProtocolGame::parseFloorChangeUp(const InputMessagePtr& msg)
{
    Position pos = g_map.getCentralPosition();
    pos.z--;

    int skip = 0;
    if(pos.z == Otc::SEA_FLOOR)
        for(int i = Otc::SEA_FLOOR - Otc::AWARE_UNDEGROUND_FLOOR_RANGE; i >= 0; i--)
            skip = setFloorDescription(msg, pos.x - Otc::AWARE_X_LEFT_TILES, pos.y - Otc::AWARE_Y_TOP_TILES, i, Otc::AWARE_X_TILES, Otc::AWARE_Y_TILES, 8 - i, skip);
    else if(pos.z > Otc::SEA_FLOOR)
        skip = setFloorDescription(msg, pos.x - Otc::AWARE_X_LEFT_TILES, pos.y - Otc::AWARE_Y_TOP_TILES, pos.z - Otc::AWARE_UNDEGROUND_FLOOR_RANGE, Otc::AWARE_X_TILES, Otc::AWARE_Y_TILES, 3, skip);

    pos.x++;
    pos.y++;
    g_map.setCentralPosition(pos);
}

void ProtocolGame::parseFloorChangeDown(const InputMessagePtr& msg)
{
    Position pos = g_map.getCentralPosition();
    pos.z++;

    int skip = 0;
    if(pos.z == Otc::UNDERGROUND_FLOOR) {
        int j, i;
        for(i = pos.z, j = -1; i <= pos.z + Otc::AWARE_UNDEGROUND_FLOOR_RANGE; ++i, --j)
            skip = setFloorDescription(msg, pos.x - Otc::AWARE_X_LEFT_TILES, pos.y - Otc::AWARE_Y_TOP_TILES, i, Otc::AWARE_X_TILES, Otc::AWARE_Y_TILES, j, skip);
    }
    else if(pos.z > Otc::UNDERGROUND_FLOOR && pos.z < Otc::MAX_Z-1)
        skip = setFloorDescription(msg, pos.x - Otc::AWARE_X_LEFT_TILES, pos.y - Otc::AWARE_Y_TOP_TILES, pos.z + Otc::AWARE_UNDEGROUND_FLOOR_RANGE, Otc::AWARE_X_TILES, Otc::AWARE_Y_TILES, -3, skip);

    pos.x--;
    pos.y--;
    g_map.setCentralPosition(pos);
}

void ProtocolGame::parseOpenOutfitWindow(const InputMessagePtr& msg)
{
    Outfit currentOutfit = getOutfit(msg);

    std::vector<std::tuple<int, std::string, int> > outfitList;
    int outfitCount = msg->getU8();
    for(int i = 0; i < outfitCount; i++) {
        int outfitId = msg->getU16();
        std::string outfitName = msg->getString();
        int outfitAddons = msg->getU8();

        outfitList.push_back(std::make_tuple(outfitId, outfitName, outfitAddons));
    }

    std::vector<std::tuple<int, std::string> > mountList;
    if(g_game.getFeature(Otc::GamePlayerMounts)) {
        int mountCount = msg->getU8();
        for(int i = 0; i < mountCount; ++i) {
            int mountId = msg->getU16(); // mount type
            std::string mountName = msg->getString(); // mount name

            mountList.push_back(std::make_tuple(mountId, mountName));
        }
    }

    g_game.processOpenOutfitWindow(currentOutfit, outfitList, mountList);
}

void ProtocolGame::parseVipAdd(const InputMessagePtr& msg)
{
    uint id = msg->getU32();
    std::string name = msg->getString();
    bool online = msg->getU8() != 0;

    g_game.processVipAdd(id, name, online);
}

void ProtocolGame::parseVipLogin(const InputMessagePtr& msg)
{
    uint id = msg->getU32();
    g_game.processVipStateChange(id, true);
}

void ProtocolGame::parseVipLogout(const InputMessagePtr& msg)
{
    uint id = msg->getU32();
    g_game.processVipStateChange(id, false);
}

void ProtocolGame::parseTutorialHint(const InputMessagePtr& msg)
{
    int id = msg->getU8(); // tutorial id
    g_game.processTutorialHint(id);
}

void ProtocolGame::parseAutomapFlag(const InputMessagePtr& msg)
{
    // ignored
    getPosition(msg); // position
    msg->getU8(); // icon
    msg->getString(); // message
}

void ProtocolGame::parseQuestLog(const InputMessagePtr& msg)
{
    std::vector<std::tuple<int, std::string, bool> > questList;
    int questsCount = msg->getU16();
    for(int i = 0; i < questsCount; i++) {
        int id = msg->getU16();
        std::string name = msg->getString();
        bool completed = msg->getU8();
        questList.push_back(std::make_tuple(id, name, completed));
    }

    g_game.processQuestLog(questList);
}

void ProtocolGame::parseQuestLine(const InputMessagePtr& msg)
{
    std::vector<std::tuple<std::string, std::string>> questMissions;
    int questId = msg->getU16();
    int missionCount = msg->getU8();
    for(int i = 0; i < missionCount; i++) {
        std::string missionName = msg->getString();
        std::string missionDescrition = msg->getString();
        questMissions.push_back(std::make_tuple(missionName, missionDescrition));
    }

    g_game.processQuestLine(questId, questMissions);
}

void ProtocolGame::parseChannelEvent(const InputMessagePtr& msg)
{
    msg->getU16(); // channel id
    msg->getString(); // player name
    msg->getU8(); // event type
}

void ProtocolGame::parseItemInfo(const InputMessagePtr& msg)
{
    std::vector<std::tuple<ItemPtr, std::string>> list;
    int size = msg->getU8();
    for(int i=0;i<size;++i) {
        ItemPtr item(new Item);
        item->setId(msg->getU16());
        item->setCountOrSubType(msg->getU8());

        std::string desc = msg->getString();
        list.push_back(std::make_tuple(item, desc));
    }

    g_lua.callGlobalField("g_game", "onItemInfo", list);
}

void ProtocolGame::parsePlayerInventory(const InputMessagePtr& msg)
{
    int size = msg->getU16();
    for(int i=0;i<size;++i) {
        msg->getU16(); // id
        msg->getU8(); // subtype
        msg->getU16(); // count
    }
}

void ProtocolGame::parseShowModalDialog(const InputMessagePtr& msg)
{
    msg->getU32(); // id
    msg->getString(); // title
    msg->getString(); // message
    int size = msg->getU8();
    for(int i=0;i<size;++i) {
        msg->getString(); // button name
        msg->getU8(); // button value
    }
    msg->getU8(); // default escape button
    msg->getU8(); // default enter button
}

void ProtocolGame::parseExtendedOpcode(const InputMessagePtr& msg)
{
    int opcode = msg->getU8();
    std::string buffer = msg->getString();

    if(opcode == 0)
        m_enableSendExtendedOpcode = true;
    else
         callLuaField("onExtendedOpcode", opcode, buffer);
}

void ProtocolGame::setMapDescription(const InputMessagePtr& msg, int x, int y, int z, int width, int height)
{
    int startz, endz, zstep;

    if(z > Otc::SEA_FLOOR) {
        startz = z - Otc::AWARE_UNDEGROUND_FLOOR_RANGE;
        endz = std::min(z + Otc::AWARE_UNDEGROUND_FLOOR_RANGE, (int)Otc::MAX_Z);
        zstep = 1;
    }
    else {
        startz = Otc::SEA_FLOOR;
        endz = 0;
        zstep = -1;
    }

    int skip = 0;
    for(int nz = startz; nz != endz + zstep; nz += zstep)
        skip = setFloorDescription(msg, x, y, nz, width, height, z - nz, skip);
}

int ProtocolGame::setFloorDescription(const InputMessagePtr& msg, int x, int y, int z, int width, int height, int offset, int skip)
{
    for(int nx = 0; nx < width; nx++) {
        for(int ny = 0; ny < height; ny++) {
            Position tilePos(x + nx + offset, y + ny + offset, z);
            if(skip == 0)
                skip = setTileDescription(msg, tilePos);
            else {
                g_map.cleanTile(tilePos);
                skip--;
            }
        }
    }
    return skip;
}

int ProtocolGame::setTileDescription(const InputMessagePtr& msg, Position position)
{
    g_map.cleanTile(position);

    bool gotEffect = 0;
    for(int stackPos=0;stackPos<256;stackPos++) {
        if(msg->peekU16()  >= 0xff00)
            return msg->getU16() & 0xff;

        if(g_game.getFeature(Otc::GameEnvironmentEffect) && !gotEffect) {
            msg->getU16(); // environment effect
            gotEffect = true;
            continue;
        }

        if(stackPos > 10)
            g_logger.traceError(stdext::format("too many things, pos=%s, stackpos=%d", stdext::to_string(position), stackPos));

        ThingPtr thing = getThing(msg);
        g_map.addThing(thing, position, -2);
    }

    return 0;
}
Outfit ProtocolGame::getOutfit(const InputMessagePtr& msg)
{
    Outfit outfit;

    int lookType = msg->getU16();
    if(lookType != 0) {
        outfit.setCategory(ThingCategoryCreature);
        int head = msg->getU8();
        int body = msg->getU8();
        int legs = msg->getU8();
        int feet = msg->getU8();
        int addons = msg->getU8();

        outfit.setId(lookType);
        outfit.setHead(head);
        outfit.setBody(body);
        outfit.setLegs(legs);
        outfit.setFeet(feet);
        outfit.setAddons(addons);
    }
    else {
        int lookTypeEx = msg->getU16();
        if(lookTypeEx == 0) {
            outfit.setCategory(ThingCategoryEffect);
            outfit.setId(13);
        }
        else {
            outfit.setCategory(ThingCategoryItem);
            outfit.setId(lookTypeEx);
        }
    }

    if(g_game.getFeature(Otc::GamePlayerMounts)) {
        int mount = msg->getU16(); // mount
        outfit.setMount(mount);
    }

    return outfit;
}

ThingPtr ProtocolGame::getThing(const InputMessagePtr& msg)
{
    ThingPtr thing;

    int id = msg->getU16();

    if(id == 0)
        stdext::throw_exception("invalid thing id");
    else if(id == Proto::UnknownCreature || id == Proto::OutdatedCreature || id == Proto::Creature)
        thing = getCreature(msg, id);
    else // item
        thing = getItem(msg, id);

    return thing;
}

ThingPtr ProtocolGame::getMappedThing(const InputMessagePtr& msg)
{
    ThingPtr thing;
    uint16 x = msg->getU16();

    if(x != 0xffff) {
        Position pos;
        pos.x = x;
        pos.y = msg->getU16();
        pos.z = msg->getU8();
        uint8 stackpos = msg->getU8();
        assert(stackpos != 255);
        thing = g_map.getThing(pos, stackpos);
        if(!thing)
            g_logger.traceError(stdext::format("no thing at pos:%s, stackpos:%d", stdext::to_string(pos), stackpos));
    } else {
        uint32 id = msg->getU32();
        thing = g_map.getCreatureById(id);
        if(!thing)
            g_logger.traceError(stdext::format("no creature with id %u", id));
    }

    return thing;
}

CreaturePtr ProtocolGame::getCreature(const InputMessagePtr& msg, int type)
{
    if(type == 0)
        type = msg->getU16();

    CreaturePtr creature;
    bool known = (type != Proto::UnknownCreature);

    if(type == Proto::OutdatedCreature || type == Proto::UnknownCreature) {
        if(known) {
            uint id = msg->getU32();
            creature = g_map.getCreatureById(id);
            if(!creature)
                g_logger.traceError("server said that a creature is known, but it's not");
        } else {
            uint removeId = msg->getU32();
            g_map.removeCreatureById(removeId);

            uint id = msg->getU32();

            int creatureType;
            if(g_game.getClientVersion() >= 910)
                creatureType = msg->getU8();
            else {
                if(id >= Proto::PlayerStartId && id < Proto::PlayerEndId)
                    creatureType = Proto::CreatureTypePlayer;
                else if(id >= Proto::MonsterStartId && id < Proto::MonsterEndId)
                    creatureType = Proto::CreatureTypeMonster;
                else
                    creatureType = Proto::CreatureTypeNpc;
            }

            std::string name = msg->getString();

            // every creature name must start with a capital letter
            if(name.length() > 0)
                name[0] = toupper(name[0]);

            if(id == m_localPlayer->getId())
                creature = m_localPlayer;
            else if(creatureType == Proto::CreatureTypePlayer) {
                // fixes a bug server side bug where GameInit is not sent and local player id is unknown
                if(m_localPlayer->getId() == 0 && name == m_localPlayer->getName())
                    creature = m_localPlayer;
                else
                    creature = PlayerPtr(new Player);
            }
            else if(creatureType == Proto::CreatureTypeMonster)
                creature = MonsterPtr(new Monster);
            else if(creatureType == Proto::CreatureTypeNpc)
                creature = NpcPtr(new Npc);
            else
                g_logger.traceError("creature type is invalid");

            if(creature) {
                creature->setId(id);
                creature->setName(name);

                g_map.addCreature(creature);
            }
        }

        int healthPercent = msg->getU8();
        Otc::Direction direction = (Otc::Direction)msg->getU8();
        Outfit outfit = getOutfit(msg);

        Light light;
        light.intensity = msg->getU8();
        light.color = msg->getU8();

        int speed = msg->getU16();
        int skull = msg->getU8();
        int shield = msg->getU8();

        // emblem is sent only when the creature is not known
        int emblem = -1;
        bool unpass = true;

        if(g_game.getFeature(Otc::GameCreatureEmblems) && !known)
            emblem = msg->getU8();

        if(g_game.getClientVersion() >= 854)
            unpass = msg->getU8();

        if(creature) {
            creature->setHealthPercent(healthPercent);
            creature->setDirection(direction);
            creature->setOutfit(outfit);
            creature->setLight(light);
            creature->setSpeed(speed);
            creature->setSkull(skull);
            creature->setShield(shield);
            if(emblem != -1)
                creature->setEmblem(emblem);
            creature->setPassable(!unpass);

            if(creature == m_localPlayer && !m_localPlayer->isKnown())
                m_localPlayer->setKnown(true);
        }
    } else if(type == Proto::Creature) {
        uint id = msg->getU32();
        creature = g_map.getCreatureById(id);

        if(!creature)
            g_logger.traceError("invalid creature");

        Otc::Direction direction = (Otc::Direction)msg->getU8();
        if(creature)
            creature->turn(direction);

        if(g_game.getClientVersion() >= 953) {
            bool unpass = msg->getU8();

            if(creature)
                creature->setPassable(!unpass);
        }

    } else {
        stdext::throw_exception("invalid creature opcode");
    }

    return creature;
}

ItemPtr ProtocolGame::getItem(const InputMessagePtr& msg, int id)
{
    if(id == 0)
        id = msg->getU16();

    ItemPtr item = Item::create(id);
    if(item->getId() == 0)
        stdext::throw_exception(stdext::format("unable to create item with invalid id %d", id));

    if(item->isStackable() || item->isFluidContainer() || item->isSplash() || item->isChargeable())
        item->setCountOrSubType(msg->getU8());

    if(g_game.getFeature(Otc::GameItemAnimationPhase)) {
        if(item->getAnimationPhases() > 1) {
            // 0xfe => random phase
            // 0xff => async?
            msg->getU8(); // phase
        }
    }

    return item;
}

Position ProtocolGame::getPosition(const InputMessagePtr& msg)
{
    uint16 x = msg->getU16();
    uint16 y = msg->getU16();
    uint8  z = msg->getU8();

    return Position(x, y, z);
}