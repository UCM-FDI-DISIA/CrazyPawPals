#include "MultiplayerHUD.h"
#include "../../../game/Game.h"
#include "../../../ecs/Manager.h"
#include "../Health.h"


void MultiplayerHUD::initComponent()
{
    _player_id = Game::Instance()->get_network_state().connections.local_user_index;
    _camera = Game::Instance()->get_mngr()->getComponent<camera_component>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA));
    _num_player_connected = Game::Instance()->get_network_state().connections.connected_users;
}

void MultiplayerHUD::render()
{
    drawText("Player " + std::to_string(_player_id + 1), 0.88, 0.05);
    float startX = 0.82;
    float startY = 0.82;
    float spacingY = 0.05;

    int act_player = 0;

    auto&& mngr = Game::Instance()->get_mngr();
    auto net_players = Game::Instance()->get_network_state().game_state.user_players;

    for (int i = 0; i < _num_player_connected; ++i)
    {
        if (i != _player_id) {
            auto player = net_players.at(i);
            float x = startX;
            float y = startY + act_player * spacingY;

            // imagen
            std::string tex_name = Game::Instance()->get_network_state().game_state.users_sprite_keys.at(i) + "_icon";
            drawIcon(tex_name, x, y);

            // vida
            auto health = mngr->getComponent<Health>(player);
            drawHealthBar(x + 0.04, y, health->getHealth(), health->getMaxHealth(), health->getShield());

            // id 
            std::string id = "Player " + std::to_string(i + 1);
            drawText(id, x + 0.045 + (0.0005 * health->getMaxHealth()), y);
            act_player++;

        }
    }
}

void MultiplayerHUD::drawIcon(const std::string& texture, float x, float y)
{
    rect_f32 icon_rect = rect_f32_screen_rect_from_viewport(rect_f32({ { x,y }, { 0.04,0.055 } }), _camera->cam.screen);
    SDL_Rect icon_true{
        int(icon_rect.position.x),
        int(icon_rect.position.y),
        int(icon_rect.size.x),
        int(icon_rect.size.y)
    };
    Texture& tex = sdlutils().images().at(texture);
    tex.render(icon_true);
}

void MultiplayerHUD::drawHealthBar(float x, float y, int health, int maxHealth, int shield)
{
    SDL_Renderer* renderer = sdlutils().renderer();
    static constexpr float scale = 0.0005;
    float _y = y + 0.01f;
    SDL_SetRenderDrawColor(renderer, 80, 55, 60, 255);
    rect_f32 health1 = rect_f32_screen_rect_from_viewport(rect_f32({ { x,_y }, { maxHealth * scale,0.03 } }), _camera->cam.screen);;
    SDL_Rect health1true{
        int(health1.position.x),
        int(health1.position.y),
        int(health1.size.x),
        int(health1.size.y)
    };
    SDL_RenderFillRect(renderer, &health1true);

    //remaining
    SDL_SetRenderDrawColor(renderer, 200, 80, 100, 255);
    rect_f32 health2 = rect_f32_screen_rect_from_viewport(rect_f32({{ x,_y }, { health * scale,0.03 }}), _camera->cam.screen);
    SDL_Rect health2true{
        int(health2.position.x),
        int(health2.position.y),
        int(health2.size.x),
        int(health2.size.y)
    };
    SDL_RenderFillRect(renderer, &health2true);

    //shield (overlaps health)
    SDL_SetRenderDrawColor(renderer, 190, 210, 255, 125);
    rect_f32 shieldrect = rect_f32_screen_rect_from_viewport(rect_f32({ { x,_y }, { shield * scale,0.04 } }), _camera->cam.screen);
    SDL_Rect shieldrecttrue{
        int(shieldrect.position.x),
        int(shieldrect.position.y),
        int(shieldrect.size.x),
        int(shieldrect.size.y)
    };
    SDL_RenderFillRect(renderer, &shieldrecttrue);
}

void MultiplayerHUD::drawText(const std::string& text, float x, float y)
{
    rect_f32 text_rect = rect_f32_screen_rect_from_viewport(rect_f32({ { x,y + 0.005f }, { 0.05,0.05 } }), _camera->cam.screen);
    SDL_Rect text_true{
        int(text_rect.position.x),
        int(text_rect.position.y),
        int(text_rect.size.x),
        int(text_rect.size.y)
    };
    Texture text_tex{
        sdlutils().renderer(),
        text,
        sdlutils().fonts().at("RUBIK_MONO"),
        SDL_Color({50,50,50,255}) };
    text_tex.render(text_true);
}
