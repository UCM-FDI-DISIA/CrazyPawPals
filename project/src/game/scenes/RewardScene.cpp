
#include "RewardScene.h"

#include "../../our_scripts/components/ui/Button.h"
#include "../GameStructs.h"
#include "../../utils/Vector2D.h"
#include "../../sdlutils/SDLUtils.h"
#include "../../sdlutils/InputHandler.h"
#include "../../ecs/Entity.h"

#include "../../our_scripts/card_system/Card.hpp"
#include "../../our_scripts/card_system/CardList.h" 
#include "../../our_scripts/card_system/PlayableCards.hpp"
#include "../../our_scripts/components/cards/Deck.hpp"
#include "../../our_scripts/components/rendering/transformless_dyn_image.h" 
#include "../../our_scripts/components/rendering/ImageForButton.h"
#include "../../our_scripts/components/cards/RewardDataComponent.h"
#include "../../our_scripts/components/Health.h" 

#ifdef GENERATE_LOG
#include "../../our_scripts/log_writer_to_csv.hpp"
#endif

#include <iostream>
bool RewardScene::_mythic = false;
float buttonX = 0.38f; 
RewardScene::RewardScene() : Scene(ecs::scene::REWARDSCENE),_selected_card(nullptr),
_heal(false), _lr(nullptr),
_selected(false), _activate_confirm_button(false), _special_case(false),_chosen_card(nullptr), _activate_exchange_button(false), _last_deck_card_img(nullptr), _activate_heal(false), _exchange(false)
{
}
RewardScene::~RewardScene()
{
}

void RewardScene::initScene() {
    create_static_background(&sdlutils().images().at("reward"));
    create_reward_info();
    create_reward_buttons();
    create_my_deck_cards();
}

void RewardScene::enterScene()
{
    auto* mngr = Game::Instance()->get_mngr();
    auto* player = mngr->getHandler(ecs::hdlr::PLAYER);
    auto _m_deck = mngr->getComponent<Deck>(player);
    auto& draw = _m_deck->move_discard_to_draw().card_list();
    refresh_my_deck_cards(draw);
    refresh_rewards();
    check_number();
    sdlutils().soundEffects().at("reward").play();
    
    Game::Instance()->get_mngr()->change_ent_scene(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA), ecs::scene::REWARDSCENE);

    if (!Game::Instance()->is_network_none()) {
        Game::network_users_state& state = Game::Instance()->get_network_state();
        state.game_state.ready_users.reset();
        state.game_state.ready_user_count = 0;
    }
#ifdef GENERATE_LOG
    log_writer_to_csv::Instance()->add_new_log();
    log_writer_to_csv::Instance()->add_new_log("ENTERED REWARDS SCENE");
#endif
}

void RewardScene::exitScene()
{
    showing_message = false;

    change_pos(false);
    auto* mngr = Game::Instance()->get_mngr();
    auto cb = mngr->getHandler(ecs::hdlr::CONFIRMREWARD);
    auto eb = mngr->getHandler(ecs::hdlr::EXCHANGEBUTTON);

    auto cImg = mngr->getComponent<ImageForButton>(cb);

    if (!_exchange && _last_deck_card_img != nullptr) {
        auto eImg = mngr->getComponent<ImageForButton>(eb);
        _last_deck_card_img->destination_rect.position.y += 0.05f;
        eImg->_filter = false;
        eImg->swap_textures();
        eImg->_filter = false;
    }
    else if (_exchange || (!_exchange && _heal)) {
        auto eImg = mngr->getComponent<ImageForButton>(eb);
        eImg->destination_rect.position.x = 10.0f;
        eImg->_filter = false;
        eImg->swap_textures();
        eImg->_filter = false;
    }

    _lr->destination_rect.size = { _lr->destination_rect.size.x /1.1f,  _lr->destination_rect.size.y/1.1f };
    
    _exchange = false;
    _lr = nullptr;
    _selected = false;
    _heal = false;

    _last_deck_card_img = nullptr;
    _selected_card = nullptr;

    _chosen_card = nullptr;

    _activate_confirm_button = false;
    _activate_exchange_button = false;
    _activate_heal = false;
#ifdef GENERATE_LOG
    log_writer_to_csv::Instance()->add_new_log("EXIT REWARDS SCENE");
    log_writer_to_csv::Instance()->add_new_log();
#endif
}

void RewardScene::render()
{
    Scene::render();
    if (showing_message) {
        auto camera = Game::Instance()->get_mngr()->getComponent<camera_component>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA));
        rect_f32 message_rect = rect_f32_screen_rect_from_viewport(rect_f32({ { 0.4,0.7 }, { 0.2,0.05 } }), camera->cam.screen);
        SDL_Rect message_true{
            int(message_rect.position.x),
            int(message_rect.position.y),
            int(message_rect.size.x),
            int(message_rect.size.y)
        };
        Texture message_tex{
        sdlutils().renderer(),
        message,
        sdlutils().fonts().at("RUBIK_MONO"),
        SDL_Color({128, 0, 32, 255}) };
        message_tex.render(message_true);
    }
}

//method to get a unique card (used to prevent repeated rewards)
std::pair<std::string, GameStructs::CardType> RewardScene::get_unique_card(std::unordered_set<std::string>& appeared_cards) {
    std::string sprite;
    GameStructs::CardType ct;
    do {
        ct = (GameStructs::CardType)sdlutils().rand().nextInt(0, GameStructs::LAST_CARD);
        sprite = select_card(ct);
    } while (appeared_cards.find(sprite) != appeared_cards.end()); // Repite si la carta ya apareci�

    appeared_cards.insert(sprite);
    return { sprite, ct };
}

//method to return a string used for the render part
std::string RewardScene::select_card(GameStructs::CardType ct) {
    std::string s = "";
    switch (ct)
    {
    case GameStructs::FIREBALL: s = "card_fireball";
        break;
    case GameStructs::LIGHTING: s = "card_lighting";
        break;
    case GameStructs::KUNAI: s = "card_kunai";
        break;
    case GameStructs::RECOVER: s = "card_recover";
        break;
    case GameStructs::MINIGUN: s = "card_minigun";
        break;
    case GameStructs::SPRAY: s = "card_cardSpray";
        break;
    case GameStructs::ELDRITCH_BLAST: s = "card_eldritchBlast";
        break;
    case GameStructs::COMMUNE: s = "card_commune";
        break;
    case GameStructs::EVOKE: s = "card_evoke";
        break;
    case GameStructs::QUICK_FEET: s = "card_quickFeet";
        break;
    case GameStructs::FULGUR: s = "card_fulgur";
        break;
    default:
        break;
    }
    //std::cout << "reward card: " + s << std::endl;
    return s;
}

void RewardScene::refresh_rewards() {
    auto* mngr = Game::Instance()->get_mngr();
    auto& rewards_cards = mngr->getEntities(ecs::grp::REWARDCARDS);
    std::unordered_set<std::string> appeared_cards;
    //refresh the three reward card button
    for (auto& e : rewards_cards) {
        auto s = get_unique_card(appeared_cards);
        auto img = mngr->getComponent<transformless_dyn_image>(e);
        img->set_texture(&sdlutils().images().at(s.first));
        auto data = mngr->getComponent<RewardDataComponent>(e);
        data->set_data(s.first, s.second);
    }

    change_pos(true);
}

void RewardScene::change_pos(bool enter) {
    //auxiliar lambda function to swap positions
    auto swap_positions = [](transformless_dyn_image* img1, transformless_dyn_image* img2) {
        auto aux = img1->destination_rect.position.x;
        img1->destination_rect.position.x = img2->destination_rect.position.x;
        img2->destination_rect.position.x = aux;
        };

#pragma region obtain references
    auto* mngr = Game::Instance()->get_mngr();
    //third reward
    auto& rewardCard = mngr->getEntities(ecs::grp::REWARDCARDS)[2];

    // players health
    auto playerHealth = mngr->getComponent<Health>(mngr->getHandler(ecs::hdlr::PLAYER));
    int act = playerHealth->getHealth();
    int max = playerHealth->getMaxHealth();

    // other references
    auto healthReward = mngr->getHandler(ecs::hdlr::REWARDHEALTH);
    auto img = mngr->getComponent<transformless_dyn_image>(rewardCard); // �ltima carta
    auto healImg = mngr->getComponent<transformless_dyn_image>(healthReward);

    auto bRewardButton = mngr->getComponent<Button>(rewardCard);
    auto hRewardButton = mngr->getComponent<Button>(healthReward);
#pragma endregion

#pragma region health reward activation

    if (enter) { //if we need to activate the heal reward
        if ((float)act / (float)max <= 0.2f) {
            swap_positions(img, healImg);
            _activate_heal = true;
        }
    }
    else { //in other case, the condition to swap changes
        //we only swap if player has chosen heal reward;
        if (_activate_heal)swap_positions(img, healImg);
    }
#pragma endregion
}

void RewardScene::create_reward_buttons() {
    float umbral = 0.2f;
    GameStructs::ButtonProperties buttonPropTemplate = {
        { {0.45f, 0.18f}, {0.125f, 0.2f} },
        0.0f, "", ecs::grp::REWARDCARDS
    };

    // Auxiliar set to check if we repeat some card
    std::unordered_set<std::string> appeared_cards;

    //three card reward buttons
    buttonPropTemplate.sprite_key = get_unique_card(appeared_cards).first;
    create_reward_card_button(buttonPropTemplate);

    buttonPropTemplate.sprite_key = get_unique_card(appeared_cards).first;
    buttonPropTemplate.rect.position.x -= umbral;
    create_reward_card_button(buttonPropTemplate);
     
    buttonPropTemplate.sprite_key = get_unique_card(appeared_cards).first;
    buttonPropTemplate.rect.position.x += umbral*2;
    create_reward_card_button(buttonPropTemplate);

    //it only appears in certain circustances (if so, we swap the position between this button and the third reward card)
    buttonPropTemplate.ID = ecs::grp::UI;
    buttonPropTemplate.sprite_key = "reward_health";
    buttonPropTemplate.rect.position.x = 20.0f;
    create_reward_health_button(buttonPropTemplate);

    //selected button
    buttonPropTemplate.ID = ecs::grp::UI;
    buttonPropTemplate.sprite_key = "confirm_reward";
    buttonPropTemplate.rect.position = { buttonX, 0.57f };
    buttonPropTemplate.rect.position.x += 10.0f;
    buttonPropTemplate.rect.size = { 0.3f, 0.2f };
    create_reward_selected_button(buttonPropTemplate);

    //exchange button
    buttonPropTemplate.ID = ecs::grp::UI;
    buttonPropTemplate.sprite_key = "exchange_reward";
    buttonPropTemplate.rect.position.x = 10.0f;
    create_reward_exchange_button(buttonPropTemplate);

    //next round button
    buttonPropTemplate.ID = ecs::grp::UI;
    buttonPropTemplate.sprite_key = "next";
    buttonPropTemplate.rect.position = { 2.0f, 0.57f };
    create_next_round_button(buttonPropTemplate);
}

void RewardScene::create_reward_health_button(const GameStructs::ButtonProperties& bp) {
    auto* mngr = Game::Instance()->get_mngr();
    auto e = create_button(bp);
    mngr->setHandler(ecs::hdlr::REWARDHEALTH, e);

    auto buttonComp = mngr->getComponent<Button>(e);
    //used for change the sprite once a button is clicked
    auto imgComp = mngr->addComponent<ImageForButton>(e,
        &sdlutils().images().at(bp.sprite_key),
        &sdlutils().images().at("reward_selected"),
        bp.rect,
        0,
        Game::Instance()->get_mngr()->getComponent<camera_component>(
            Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA))->cam
    );
    mngr->addComponent<RewardDataComponent>(e, bp.sprite_key);
    auto ri = mngr->getComponent<transformless_dyn_image>(mngr->getHandler(ecs::hdlr::REWARDINFO));

    buttonComp->connectClick([buttonComp, imgComp, this]() {
        if (_selected) {
            std::cout << "already selected" << std::endl;
        }
        else {
            std::cout << "left click -> health button" << std::endl;
            if (_lr == nullptr) {
                imgComp->resize(1.1f);
            }
            else if (_lr != nullptr && _lr != imgComp) {
                imgComp->resize(1.1f);
                _lr->resize(1.0f / 1.1f);
            }
            _lr = imgComp;
            _heal = true;
            _activate_confirm_button = true;
            _activate_exchange_button = false;
            _special_case = false;
            _chosen_card = nullptr;
        }
    });
    buttonComp->connectHover([mngr, buttonComp, imgComp, ri, e,this]() {
        if (_selected) return;
        //std::cout << "hover -> Reward button: " << std::endl;
        //filter
        imgComp->_filter = true;
        auto& sp = mngr->getComponent<RewardDataComponent>(e)->sprite();
        ri->set_texture(&sdlutils().images().at(sp + "_info"));
        sdlutils().soundEffects().at("button_hover").play();
    });
    buttonComp->connectExit([buttonComp, imgComp, ri]() {
        //std::cout << "exit -> Reward button: " << std::endl;
        //filter
        imgComp->_filter = false;
        ri->set_texture(&sdlutils().images().at("initial_info"));
        
    });
}

void RewardScene::create_reward_card_button(const GameStructs::ButtonProperties& bp)
{
    auto* mngr = Game::Instance()->get_mngr();
    auto e = create_button(bp);
    auto buttonComp = mngr->getComponent<Button>(e);
    
    //used for change the sprite once a button is clicked
    auto imgComp = mngr->addComponent<ImageForButton>(e,
        &sdlutils().images().at(bp.sprite_key), 
        &sdlutils().images().at("reward_selected"),
        bp.rect, 
        0, 
        Game::Instance()->get_mngr()->getComponent<camera_component>( 
            Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA))->cam 
    );  
    //used to get the sprite key for show a specific info
    mngr->addComponent<RewardDataComponent>(e, bp.sprite_key);
    auto ri = mngr->getComponent<transformless_dyn_image>(mngr->getHandler(ecs::hdlr::REWARDINFO));

    buttonComp->connectClick([buttonComp, imgComp, this, e]() {
        if (_selected) {
            //std::cout << "already selected" << std::endl;
        }
        else {
            //std::cout << "left click -> reward card button" << std::endl;
            if (_lr == nullptr) {
                imgComp->resize(1.1f);
            }
            else if (_lr != nullptr && _lr != imgComp) {
                imgComp->resize(1.1f);
                _lr->resize(1.0f/1.1f);
            }
            _lr = imgComp;
            _chosen_card = e;
            _heal = false;
            check_number();
        }
    });
    buttonComp->connectHover([mngr,buttonComp, imgComp, ri, e,this]() {
        if (_selected) return;
        //std::cout << "hover -> Reward button: " << std::endl;
        //filter
        imgComp->_filter = true;
        auto& sp = mngr->getComponent<RewardDataComponent>(e)->sprite();
        ri->set_texture(&sdlutils().images().at(sp + "_info"));
        sdlutils().soundEffects().at("button_hover").play();

        });
    buttonComp->connectExit([buttonComp, imgComp, ri]() {
        //std::cout << "exit -> Reward button: " << std::endl;
        //filter
        imgComp->_filter = false;
        ri->set_texture(&sdlutils().images().at("initial_info"));
        });

}

ecs::entity_t RewardScene::create_card_button(const GameStructs::CardButtonProperties& bp) {
    auto* mngr = Game::Instance()->get_mngr();
    auto e = create_button(bp);
    auto b = mngr->addComponent<CardButton>(e);
    b->initComponent();
    b->set_it(bp.iterator);
    return e;
}

void RewardScene::create_a_deck_card(const GameStructs::CardButtonProperties& bp) {
    auto* mngr = Game::Instance()->get_mngr();

    auto e = create_card_button(bp);
    auto buttonComp = mngr->getComponent<CardButton>(e);
    auto imgComp = mngr->getComponent<transformless_dyn_image>(e);
    auto ri = mngr->getComponent<transformless_dyn_image>(mngr->getHandler(ecs::hdlr::REWARDINFO));
    mngr->addComponent<RewardDataComponent>(e, bp.sprite_key);

    buttonComp->connectClick([buttonComp, imgComp, this, bp] {
        if (_selected || _heal) return;
        auto it = buttonComp->It();
        if (it == nullptr || !it->can_be_replaced()) return;
        imgComp->destination_rect.size = { imgComp->_original_w,  imgComp->_original_h };
        //only assign a valid iterator
        if (it != _selected_card) {

            _selected_card = it;
            imgComp->destination_rect.position.y -= 0.05f;
            if (_last_deck_card_img != nullptr) _last_deck_card_img->destination_rect.position.y += 0.05f;
            _last_deck_card_img = static_cast<ImageForButton*>(imgComp);
            //std::cout << "card selected: "<< std::endl;
        }
        });

    buttonComp->connectHover([buttonComp, imgComp, mngr, ri, e, this]() {
        //std::cout << "hover -> Reward button: " << std::endl;
        //filter
        if (_selected) return;
        imgComp->_filter = true;
        auto& sp = buttonComp->Name();
        if (sp != "") ri->set_texture(&sdlutils().images().at(sp + "_info"));
        sdlutils().soundEffects().at("button_hover").play();
        /*imgComp->destination_rect.position.y -= 0.125f;*/
       /* imgComp->destination_rect.size = { imgComp->destination_rect.size.x * 1.25f,  imgComp->destination_rect.size.y * 1.25f };*/
        });
    buttonComp->connectExit([buttonComp, imgComp, mngr, ri]() {
        //std::cout << "exit -> Reward button: " << std::endl;
        /*imgComp->destination_rect.position.y += 0.125f;*/
        //filter
        imgComp->_filter = false;
        ri->set_texture(&sdlutils().images().at("initial_info"));
        //imgComp->destination_rect.size = { imgComp->destination_rect.size.x / 1.25f,  imgComp->destination_rect.size.y / 1.25f };
        });
}

void RewardScene::create_my_deck_cards() {
    auto* mngr = Game::Instance()->get_mngr();

    //GET PLAYERS DECK REFERENCE
    auto* player = mngr->getHandler(ecs::hdlr::PLAYER);
    if (player && !mngr->hasComponent<Deck>(player)) {
        //when we add these entities, our olayer doesnt have any deck as component
        mngr->addComponent<Deck>(player);
    }
    auto _m_deck = mngr->getComponent<Deck>(player);
    auto& pDeck = _m_deck->move_discard_to_draw(true).card_list();

    float umbral = 0.09f;
    auto iterator = pDeck.begin();
    GameStructs::CardButtonProperties propTemplate = {
        { {0.01f, 0.8f}, {0.075f, 0.135f} },
        0.0f, "", ecs::grp::REWARDDECK, *iterator
    };

    for (const auto& it : pDeck) {
#pragma region convert a class name to a string
        std::string typeName = it->get_name();
#pragma endregion
        propTemplate.sprite_key = typeName;
        create_a_deck_card(propTemplate);
        propTemplate.rect.position.x += umbral;
        iterator++;
    }
    propTemplate.iterator = nullptr;
    for (int i = 0; i < 6; ++i) {
        propTemplate.sprite_key = "initial_info";
        create_a_deck_card(propTemplate);
        propTemplate.rect.position.x += umbral;
    }
}

void RewardScene::refresh_my_deck_cards(const std::list<Card*>& cl) {
    auto* mngr = Game::Instance()->get_mngr();
    auto& infos = mngr->getEntities(ecs::grp::REWARDDECK);

    if (infos.empty()) return;

    auto itRewardInfo = infos.begin();
    //refresh my deck info and represent it
    for (auto& c : cl) {
        //obtain each ones component
        auto img = mngr->getComponent<transformless_dyn_image>(*itRewardInfo);
       
        std::string typeName = c->get_name();

        //change to the newest texture
        img->set_texture(&sdlutils().images().at(typeName));
        //Refresh the pointer saved in the component
        auto buttonComp = mngr->getComponent<Button>(*itRewardInfo);
        if (buttonComp) {
            static_cast<CardButton*>(buttonComp)->set_it(c);
            static_cast<CardButton*>(buttonComp)->set_name(typeName);
        }
        ++itRewardInfo;
    }

    //refresh rest of the deck infos (blank infos)
    for (; itRewardInfo != infos.end(); ++itRewardInfo) {
        auto img = mngr->getComponent<transformless_dyn_image>(*itRewardInfo);
            img->set_texture(&sdlutils().images().at("initial_info"));

            auto buttonComp = mngr->getComponent<Button>(*itRewardInfo);
            if (buttonComp) {
                static_cast<CardButton*>(buttonComp)->set_it(nullptr); // Poner puntero a nullptr si no hay carta
            }
    }
}

void RewardScene::create_reward_selected_button(const GameStructs::ButtonProperties& bp) {
    auto* mngr = Game::Instance()->get_mngr();

    auto e = create_button(bp);
    auto buttonComp = mngr->getComponent<Button>(e);
    auto imgComp = mngr->addComponent<ImageForButton>(e,
        &sdlutils().images().at(bp.sprite_key),
        &sdlutils().images().at(bp.sprite_key+"_selected"),
        bp.rect,
        0,
        Game::Instance()->get_mngr()->getComponent<camera_component>(
            Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA))->cam
    );

    mngr->setHandler(ecs::hdlr::CONFIRMREWARD,e);
    buttonComp->connectClick([buttonComp, this, mngr,imgComp] {
        if (_heal) {
            auto* mngr = Game::Instance()->get_mngr();
            auto player = mngr->getHandler(ecs::hdlr::PLAYER);
            auto phealth = mngr->getComponent<Health>(player);
            //heal a 50%
            int hn = phealth->getMaxHealth() * 5 / 10;
            phealth->heal(hn);
            _lr->swap_textures();
        }
        //we only select a reward if previously we have chosen something
        else if (_lr != nullptr && !_selected) {
            _lr->swap_textures();
            add_new_reward_card();
        }
        _selected = true;
        auto imgNext = mngr->getComponent<ImageForButton>(mngr->getHandler(ecs::hdlr::NEXTROUNDREWARD));
        imgComp->_filter = false;
        imgComp->swap_textures();
        imgNext->destination_rect.position = { buttonX, 0.4f };
        imgComp->destination_rect.position.x += 100.0f;
    });
    buttonComp->connectHover([buttonComp, imgComp, this]() {
        if (_selected) return;
        //std::cout << "hover -> Reward selected button: " << std::endl;
        //filter
        sdlutils().soundEffects().at("button_hover").play();
        imgComp->swap_textures();
        imgComp->_filter = true;
    });
    buttonComp->connectExit([buttonComp, imgComp, this]() {
        //std::cout << "exit -> Reward selected button: " << std::endl;
        //filter
        if (_selected) return;
        imgComp->_filter = false;
        imgComp->swap_textures();
    });
}

void RewardScene::remove_deck_card()
{
    auto mngr = Game::Instance()->get_mngr();
    auto player = mngr->getHandler(ecs::hdlr::PLAYER);
    auto deck = mngr->getComponent<Deck>(player);
    deck->remove_card(_selected_card);
}

void RewardScene::add_new_reward_card() {
    assert(_chosen_card != nullptr);

    //Once reward is picked, checks number of cards in deck
    auto* mngr = Game::Instance()->get_mngr();
    auto* player = mngr->getHandler(ecs::hdlr::PLAYER);
    auto _m_deck = mngr->getComponent<Deck>(player);
    //We add the cards to the deck
    Card* c = nullptr;
    GameStructs::CardType ct = mngr->getComponent<RewardDataComponent>(_chosen_card)->CT();
    switch (ct)
    {
    case GameStructs::FIREBALL:
        c = new Fireball();
        break;
    case GameStructs::LIGHTING:
        c = new Lighting();
        break;
    case GameStructs::KUNAI:
        c = new Kunai();
        break;
    case GameStructs::RECOVER:
        c = new Recover();
        break;
    case GameStructs::MINIGUN:
        c = new Minigun();
        break;
    case GameStructs::SPRAY:
        c = new CardSpray();
        break;
    case GameStructs::ELDRITCH_BLAST:
        c = new EldritchBlast();
        break;
    case GameStructs::COMMUNE:
        c = new Commune();
        break;
    case GameStructs::EVOKE:
        c = new Evoke();
        break;
    case GameStructs::FULGUR:
        c = new Fulgur();
        break;
    case GameStructs::QUICK_FEET:
        c = new QuickFeet();
        break;
    default:
        break;
    }
    if (c != nullptr) {
        _m_deck->add_card_to_deck(c);
        //Refresh deck
        auto& pDeck = _m_deck->move_discard_to_draw().card_list();
        refresh_my_deck_cards(pDeck);
        
    }
}

void RewardScene::check_number()
{
    //Once reward is picked, checks number of cards in deck
    auto* mngr = Game::Instance()->get_mngr();
    auto* player = mngr->getHandler(ecs::hdlr::PLAYER);
    auto _m_deck = mngr->getComponent<Deck>(player);
    auto& pDeck = _m_deck->move_discard_to_draw().card_list();
    if (pDeck.size() < 6)
    {
        _activate_exchange_button = false;
        _activate_confirm_button = true;
    }
    else if (pDeck.size() >= 6 && pDeck.size() < 10)
    {
        _activate_exchange_button = true;
        _activate_confirm_button = true;
    }
    else if (pDeck.size() == 10)
    {
        _activate_confirm_button = false;
        _activate_exchange_button = false;
        _special_case = true;

    }
}

void RewardScene::create_reward_exchange_button(const GameStructs::ButtonProperties& bp)
{
    auto* mngr = Game::Instance()->get_mngr();

    auto e = create_button(bp);
    auto buttonComp = mngr->getComponent<Button>(e);
    auto imgComp = mngr->addComponent<ImageForButton>(e,
        &sdlutils().images().at(bp.sprite_key),
        &sdlutils().images().at(bp.sprite_key+"_selected"),
        bp.rect,
        0,
        Game::Instance()->get_mngr()->getComponent<camera_component>(
            Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA))->cam
    );
    mngr->setHandler(ecs::hdlr::EXCHANGEBUTTON, e);
    buttonComp->connectClick([buttonComp, mngr, imgComp,this] 
        {
            //if we dont have enough card to exchange, ignore this callback
            if (_selected || _activate_exchange_button || _selected_card == nullptr) return;
            _lr->_filter = false;
            _lr->swap_textures();
            _selected = true;
            _exchange = true;
            remove_deck_card();
            add_new_reward_card();
            _last_deck_card_img->destination_rect.position.y += 0.05f;

            auto imgNext = mngr->getComponent<ImageForButton>(mngr->getHandler(ecs::hdlr::NEXTROUNDREWARD));
            imgNext->destination_rect.position = { buttonX, 0.4f };

            auto imgConfirm = mngr->getComponent<ImageForButton>(mngr->getHandler(ecs::hdlr::CONFIRMREWARD));

            imgConfirm->destination_rect.position.x += 100.0f;
            imgComp->destination_rect.position.x += 100.0f;
        });
    buttonComp->connectHover([buttonComp, imgComp, this]() {
        if (_selected) return;
        sdlutils().soundEffects().at("button_hover").play();
        imgComp->swap_textures();
        imgComp->_filter = true;
        });
    buttonComp->connectExit([buttonComp, imgComp, this]() {
        imgComp->_filter = false;
        imgComp->swap_textures();
        });
}

void RewardScene::update(uint32_t delta_time) {
   Scene::update(delta_time);

   if (_activate_confirm_button && _lr != nullptr) {
       auto mngr = Game::Instance()->get_mngr();
       auto imgCompConfirm = mngr->getComponent<ImageForButton>(mngr->getHandler(ecs::hdlr::CONFIRMREWARD));
       auto imgCompExchange = mngr->getComponent<ImageForButton>(mngr->getHandler(ecs::hdlr::EXCHANGEBUTTON));
       imgCompConfirm->destination_rect.position.x = buttonX; 
       imgCompExchange->destination_rect.position.x = buttonX + 10.0f;
       _activate_confirm_button = false;
   }
   else if (_activate_exchange_button && _lr != nullptr && _selected_card != nullptr) {
       auto mngr = Game::Instance()->get_mngr();
       auto imgCompConfirm = mngr->getComponent<ImageForButton>(mngr->getHandler(ecs::hdlr::CONFIRMREWARD));
       auto imgCompExchange = mngr->getComponent<ImageForButton>(mngr->getHandler(ecs::hdlr::EXCHANGEBUTTON));
       imgCompExchange->destination_rect.position.x = buttonX - 0.185f;
       imgCompConfirm->destination_rect.position.x = buttonX + 0.185f;
       _activate_exchange_button = false;
   }
   else if (_special_case && _lr != nullptr) {
       auto mngr = Game::Instance()->get_mngr();
       auto imgCompConfirm = mngr->getComponent<ImageForButton>(mngr->getHandler(ecs::hdlr::CONFIRMREWARD));
       imgCompConfirm->destination_rect.position.x = buttonX + 10.0f;

       if (_selected_card != nullptr) {
           auto imgCompExchange = mngr->getComponent<ImageForButton>(mngr->getHandler(ecs::hdlr::EXCHANGEBUTTON));
           imgCompExchange->destination_rect.position.x = buttonX;
           _special_case = false;
       }
   }


   if (!Game::Instance()->is_network_none()) {
       network_context& network = Game::Instance()->get_network();

       if (Game::Instance()->is_host()) {
           if (SDLNet_CheckSockets(network.profile.host.clients_host_set, 0) > 0) {
               for (network_connection_size i = 0; i < network.profile.host.sockets_to_clients.connection_count; ++i) {
                   TCPsocket& connection = network.profile.host.sockets_to_clients.connections[i];
                   if (SDLNet_SocketReady(connection)) {
                       network_message_dynamic_pack dyn_message = network_message_dynamic_pack_receive(connection);
                       const uint16_t type_n{ dyn_message->header.type_n };
                       const uint16_t type_h{ SDLNet_Read16(&type_n) };
                       switch (type_h) {
                       case network_message_type_player_ready: {
                           auto message = network_message_dynamic_pack_into<network_message_player_id>(std::move(dyn_message));
                           auto&& payload = message->payload.content;
                           uint32_t id = SDLNet_Read32(&payload.id_n);

                           Game::network_users_state& state = Game::Instance()->get_network_state();
                           state.game_state.ready_users.set(id, true);
                           state.game_state.ready_user_count++;

                           if (checkAllPlayersReady()) {
                               if (_mythic) Game::Instance()->queue_scene(Game::MYTHICSCENE);
                               else Game::Instance()->queue_scene(Game::GAMESCENE);
                           }
                           break;
                       }
                       default: {
                           break;
                       }
                       }
                   }
               }
           }
       }
       else {
           int active_sockets = SDLNet_CheckSockets(network.profile.client.client_set, 0);
           if (active_sockets > 0 && SDLNet_SocketReady(network.profile.client.socket_to_host)) {
               auto msg = network_message_dynamic_pack_receive(network.profile.client.socket_to_host);
               const uint16_t type_n{ msg->header.type_n };
               const uint16_t type_h{ SDLNet_Read16(&type_n) };
               switch (type_h) {
               case network_message_type_start_game: {

                   if (_mythic) Game::Instance()->queue_scene(Game::MYTHICSCENE);
                   else Game::Instance()->queue_scene(Game::GAMESCENE);
                   break;
               }
               default: break;
               }
           }

       }

   }
}
void RewardScene::create_next_round_button(const GameStructs::ButtonProperties& bp) {
    auto* mngr = Game::Instance()->get_mngr();
    auto e = create_button(bp);
    mngr->setHandler(ecs::hdlr::NEXTROUNDREWARD, e);
    auto imgComp = mngr->addComponent<ImageForButton>(e,
        &sdlutils().images().at(bp.sprite_key),
        &sdlutils().images().at(bp.sprite_key + "_selected"),
        bp.rect,
        0,
        Game::Instance()->get_mngr()->getComponent<camera_component>(
            Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA))->cam
    );
    auto buttonComp = mngr->getComponent<Button>(e);

    buttonComp->connectClick([buttonComp, mngr, imgComp, this]() { if (_selected) {
        _lr->swap_textures();
        
        if (!Game::Instance()->is_network_none()) {
            network_context& network = Game::Instance()->get_network();
            Game::network_users_state& state = Game::Instance()->get_network_state();
            uint32_t id = state.connections.local_user_index;
            if (Game::Instance()->is_host()) {
                state.game_state.ready_users.set(0, true);
                state.game_state.ready_user_count++;
                if (checkAllPlayersReady()) {
                    if (_mythic) Game::Instance()->queue_scene(Game::MYTHICSCENE);
                    else Game::Instance()->queue_scene(Game::GAMESCENE);
                }
            }
            else if (!state.game_state.ready_users.test(id)) {
                std::cout << "Player id ready: " << std::to_string(id) << std::endl;
                state.game_state.ready_users.set(id, true);
                network_message_pack_send(
                    network.profile.client.socket_to_host,
                    network_message_pack_create(network_message_type_player_ready,
                        create_player_id_message(id))
                );
            }
            show_message("Esperando a los otros michis...", 5 * 1000);
        }
        else {
            if (_mythic) Game::Instance()->queue_scene(Game::MYTHICSCENE);
            else Game::Instance()->queue_scene(Game::GAMESCENE);
        }
        imgComp->destination_rect.position.x = 10.0f;
        imgComp->swap_textures();
        imgComp->_filter = false;
    }});
    buttonComp->connectHover([buttonComp, imgComp, this]() { 
        imgComp->swap_textures();
        sdlutils().soundEffects().at("button_hover").play();
        imgComp->_filter = true;
        });
    buttonComp->connectExit([buttonComp, imgComp, this]() { 
        imgComp->_filter = false; 
        imgComp->swap_textures();});
}

void RewardScene::create_reward_info() {
    auto e = create_entity(ecs::grp::UI,
        _scene_ID,
        new transformless_dyn_image({ { 0.315f,0.375f }, {0.4f,0.2f} },
            0,
            Game::Instance()->get_mngr()->getComponent<camera_component>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA))->cam,
            &sdlutils().images().at("initial_info")));
    Game::Instance()->get_mngr()->setHandler(ecs::hdlr::REWARDINFO, e);
}
