#include "Config.h"
#include "cJSON.h"
#include "io/Threads.h"
#include <ui/Main.h>
#include <ui/Components.h>
#include <ui/Presets.h>
#include <Colors.h>
#include <stdio.h>

bool playerlist_update(SDL_Renderer* renderer, struct _Component* component)
{
    PlayerList* list = (PlayerList*)component;
    SDL_Rect src = { 4528, 0, 144, 176 };
    SDL_Rect dst = { list->x * INTERFACE_SCALE, list->y * INTERFACE_SCALE, list->w * INTERFACE_SCALE, list->h * INTERFACE_SCALE };

    SDL_Point mouse;
    float scale_x, scale_y;
    Uint32 flags = SDL_GetMouseState(&mouse.x, &mouse.y);
    SDL_RenderGetScale(renderer, &scale_x, &scale_y);

    mouse.x /= scale_x;
    mouse.y /= scale_y;

    if (!(flags & SDL_BUTTON(1)))
        list->clicked = false;

    SDL_RenderCopy(renderer, g_textureSheet, &src, &dst);

    SDL_Rect title_src = (SDL_Rect){ 4672, 0, 80, 8 };
    SDL_Rect title_dst = (SDL_Rect){ (list->x + 32) * INTERFACE_SCALE, (list->y + 6) * INTERFACE_SCALE, 80 * INTERFACE_SCALE, 8 * INTERFACE_SCALE };
    SDL_RenderCopy(renderer, g_textureSheet, &title_src, &title_dst);

    Label label = LabelCreate(list->x + 8, list->y + 8, "", INTERFACE_SCALE);
    PlayerButton op = (PlayerButton){ 4672, 16, 15, 8, button_update, list->x + 20 * 2, list->y - 2 + 8, 15, 8, ui_button_op, 0, { 0 } };
    PlayerButton kick = (PlayerButton){ 4672, 24, 29, 8, button_update, list->x + 31 * 2, list->y - 2 + 8, 29, 8, ui_button_kick, 0, { 0 } };
    PlayerButton ban = (PlayerButton){ 4672, 32, 24, 8, button_update, list->x + 49 * 2, list->y - 2 + 8, 24, 8, ui_button_ban, 0, { 0 } };

    for(int i = 0; i < 7; i++)
    {
        if(list->peers[i].nickname.len == 0)
            continue;

        label.y += 8 * INTERFACE_SCALE;
        op.d_y += 8 * INTERFACE_SCALE;
        kick.d_y += 8 * INTERFACE_SCALE;
        ban.d_y += 8 * INTERFACE_SCALE;

        SDL_Rect check = (SDL_Rect){ (label.x - 2) * INTERFACE_SCALE, (label.y - 4) * INTERFACE_SCALE, 132 * INTERFACE_SCALE, 12 * INTERFACE_SCALE };
        char text[128];

        if(!list->clicked && SDL_PointInRect(&mouse, &check))
        {
            SDL_SetRenderDrawColor(renderer, 73, 0, 0, 255);
            SDL_RenderFillRect(renderer, &check);

            snprintf(text, 128, "%s", "...");
            op.peer = list->peers[i];
            kick.peer = list->peers[i];
            ban.peer = list->peers[i];

            if (!op.update(renderer, (Component*)&op) || !kick.update(renderer, (Component*)&kick) || !ban.update(renderer, (Component*)&ban))
            {
                list->clicked = true;
                break;
            }
        }
        else
            snprintf(text, 128, "%s", list->peers[i].nickname.value);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        label.text = text;
        label.update(renderer, (Component*)&label);
    }

    return true;
}

bool playerlist_bans_update(SDL_Renderer* renderer, struct _Component* component)
{
    PlayerListConfig* list = (PlayerListConfig*)component;
    SDL_Rect src = { 4528, 0, 144, 176 };
    SDL_Rect dst = { list->x * INTERFACE_SCALE, list->y * INTERFACE_SCALE, list->w * INTERFACE_SCALE, list->h * INTERFACE_SCALE };

    SDL_Point mouse;
    float scale_x, scale_y;
    Uint32 flags = SDL_GetMouseState(&mouse.x, &mouse.y);
    SDL_RenderGetScale(renderer, &scale_x, &scale_y);

    mouse.x /= scale_x;
    mouse.y /= scale_y;

    if (!(flags & SDL_BUTTON(1)))
        list->clicked = false;

    SDL_RenderCopy(renderer, g_textureSheet, &src, &dst);

    SDL_Rect title_src = (SDL_Rect){ 4763, 0, 58, 8 };
    SDL_Rect title_dst = (SDL_Rect){ (list->x + 42) * INTERFACE_SCALE, (list->y + 6) * INTERFACE_SCALE, 58 * INTERFACE_SCALE, 8 * INTERFACE_SCALE };
    SDL_RenderCopy(renderer, g_textureSheet, &title_src, &title_dst);

    Label label = LabelCreate(list->x + 8, list->y + 8, "", INTERFACE_SCALE);
    DeleteButton delete = (DeleteButton){ 4672, 40, 43, 8, button_update, list->x + 20 * 2, list->y - 2 + 8, 43, 8, ui_update_delete, false, BANS_FILE, g_bans, NULL };

    MutexLock(g_banMut);
    {
        if (SDL_PointInRect(&mouse, &dst))
        {
            list->page -= g_mouseWheel;
            list->page = SDL_clamp(list->page, 0, SDL_max((int)ceil(cJSON_GetArraySize(g_bans) / 9.0) - 1, 0));
        }

        size_t i = 0;
        for(cJSON* ban = g_bans->child; ban; ban = ban->next)
        {
            if (!ban)
                continue;

            if (i < list->page * 9)
            {
                i++;
                continue;
            }
            else if (i >= list->page * 9 + 9)
                break;

            label.y += 8 * INTERFACE_SCALE;
            delete.d_y += 8 * INTERFACE_SCALE;

            SDL_Rect check = (SDL_Rect){ (label.x - 2) * INTERFACE_SCALE, (label.y - 4) * INTERFACE_SCALE, 132 * INTERFACE_SCALE, 12 * INTERFACE_SCALE };
            
            if(!list->clicked && SDL_PointInRect(&mouse, &check))
            {                
                SDL_SetRenderDrawColor(renderer, 73, 0, 0, 255);
                SDL_RenderFillRect(renderer, &check);
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                
                label.text = "...";
                label.update(renderer, (Component*)&label);

                delete.key = ban->string;
                if (!delete.update(renderer, (Component*)&delete))
                {
                    list->clicked = true;
                    break;
                }
            }
            else
            {
                char text[128];
                snprintf(text, 128, "%s" CLRCODE_RST " (%s)", cJSON_GetStringValue(ban), strstr(ban->string, "-") != NULL ? "udid" : "ip");
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                
                label.text = text;
                label.update(renderer, (Component*)&label);
            }
            
            i++;
        }
    }
    MutexUnlock(g_banMut);

    return true;
}

bool playerlist_op_update(SDL_Renderer* renderer, struct _Component* component)
{
    PlayerListConfig* list = (PlayerListConfig*)component;
    SDL_Rect src = { 4528, 0, 144, 176 };
    SDL_Rect dst = { list->x * INTERFACE_SCALE, list->y * INTERFACE_SCALE, list->w * INTERFACE_SCALE, list->h * INTERFACE_SCALE };

    SDL_Point mouse;
    float scale_x, scale_y;
    Uint32 flags = SDL_GetMouseState(&mouse.x, &mouse.y);
    SDL_RenderGetScale(renderer, &scale_x, &scale_y);

    mouse.x /= scale_x;
    mouse.y /= scale_y;

    if (!(flags & SDL_BUTTON(1)))
        list->clicked = false;

    SDL_RenderCopy(renderer, g_textureSheet, &src, &dst);

    SDL_Rect title_src = (SDL_Rect){ 4672, 8, 96, 8 };
    SDL_Rect title_dst = (SDL_Rect){ (list->x + 24) * INTERFACE_SCALE, (list->y + 6) * INTERFACE_SCALE, 96 * INTERFACE_SCALE, 8 * INTERFACE_SCALE };
    SDL_RenderCopy(renderer, g_textureSheet, &title_src, &title_dst);

    Label label = LabelCreate(list->x + 8, list->y + 8, "", INTERFACE_SCALE);
    DeleteButton delete = (DeleteButton){ 4672, 40, 43, 8, button_update, list->x + 20 * 2, list->y - 2 + 8, 43, 8, ui_update_delete, false, OPERATORS_FILE, g_ops, NULL };

    MutexLock(g_opMut);
    {        
        if (SDL_PointInRect(&mouse, &dst))
        {
            list->page -= g_mouseWheel;
            list->page = SDL_clamp(list->page, 0, SDL_max((int)ceil(cJSON_GetArraySize(g_ops) / 9.0) - 1, 0));
        }

        size_t i = 0;
        for (cJSON* op = g_ops->child; op; op = op->next)
        {
            if (!op)
                continue;

            if (i < list->page * 9)
            {
                i++;
                continue;
            }
            else if (i >= list->page * 9 + 9)
                break;

            label.y += 8 * INTERFACE_SCALE;
            delete.d_y += 8 * INTERFACE_SCALE;

            SDL_Rect check = (SDL_Rect){ (label.x - 2) * INTERFACE_SCALE, (label.y - 4) * INTERFACE_SCALE, 132 * INTERFACE_SCALE, 12 * INTERFACE_SCALE };
            char text[128];

            if(!list->clicked && SDL_PointInRect(&mouse, &check))
            {                
                SDL_SetRenderDrawColor(renderer, 73, 0, 0, 255);
                SDL_RenderFillRect(renderer, &check);
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                
                label.text = "...";
                label.update(renderer, (Component*)&label);

                delete.key = op->string;
                if (!delete.update(renderer, (Component*)&delete))
                {
                    list->clicked = true;
                    break;
                }
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                label.text = cJSON_GetStringValue(op);
                label.update(renderer, (Component*)&label);
            }

            i++;
        }
    }
    MutexUnlock(g_opMut);

    return true;
}