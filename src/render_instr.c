#include "render_instr.h"

bool render_instr_query_output_xy(
    SDL_Renderer *renderer,
    render_instr_dest_origin dest_origin,
    int *x,
    int *y)
{
    int w, h;
    SDL_GetRendererOutputSize(renderer, &w, &h);

    switch (dest_origin)
    {
        case TOP_CENTER:
        case MIDDLE_CENTER:
        case BOTTOM_CENTER:
            *x += w / 2;
            break;

        case TOP_RIGHT:
        case MIDDLE_RIGHT:
        case BOTTOM_RIGHT:
            *x += w;
            break;
    }

    switch (dest_origin) 
    {
        case MIDDLE_LEFT:
        case MIDDLE_CENTER:
        case MIDDLE_RIGHT:
            *y += h / 2;
            break;

        break;
        case BOTTOM_LEFT:
        case BOTTOM_CENTER:
        case BOTTOM_RIGHT:
            *y += h;
            break;
    }
}

bool render_instr_set_s(
    render_instr *instr,
    SDL_Surface *surface)
{
    instr->surface = surface;
    instr->flags |= instr->dest[1].w ? 0x1 : 0x7;
}

bool render_instr_set_swh(
    render_instr *instr,
    SDL_Surface *surface,
    int w,
    int h)
{
    instr->surface = surface;
    instr->dest[1].w = w;
    instr->dest[1].h = h;
    instr->flags |= 0x7;
}

bool render_instr_set_xy(
    render_instr *instr,
    int x,
    int y)
{
    instr->dest[1].x = x;
    instr->dest[1].y = y;
    instr->flags |= 0x2;
}

bool render_instr_update(
    render_instr *instr)
{
    instr->flags |= 0x80;
}

bool render_instr_render(
    render_instr *instr,
    SDL_Renderer *renderer)
{
    if (instr->flags & 0x80)
    {
        instr->flags &= ~0x80;

        if (instr->flags & 0x1)
        {
            instr->flags &= ~0x1;

            if (instr->texture)
                SDL_DestroyTexture(instr->texture);
            
            SDL_Surface *surface = instr->surface;
            instr->texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);

            if (instr->alpha)
                SDL_SetTextureAlphaMod(instr->texture, instr->alpha);
        }

        if (instr->flags & 0x4)
        {
            instr->flags &= ~0x4;

            if (instr->dest[1].w)
            {
                instr->dest->w = instr->dest[1].w;
                instr->dest->h = instr->dest[1].h;
            }
            else if (instr->texture)
                SDL_QueryTexture(instr->texture, NULL, NULL, &instr->dest->w, &instr->dest->h);
        }

        if (instr->flags & 0x2)
        {
            instr->flags &= ~0x2;

            instr->dest->x = instr->dest[1].x;
            instr->dest->y = instr->dest[1].y;

            switch (instr->dest_origin)
            {
                case TOP_CENTER:
                case MIDDLE_CENTER:
                case BOTTOM_CENTER:
                    instr->dest->x -= instr->dest->w / 2;
                    break;

                case TOP_RIGHT:
                case MIDDLE_RIGHT:
                case BOTTOM_RIGHT:
                    instr->dest->x -= instr->dest->w;
                    break;
            }

            switch (instr->dest_origin) 
            {
                case MIDDLE_LEFT:
                case MIDDLE_CENTER:
                case MIDDLE_RIGHT:
                    instr->dest->y -= instr->dest->h / 2;
                    break;

                break;
                case BOTTOM_LEFT:
                case BOTTOM_CENTER:
                case BOTTOM_RIGHT:
                    instr->dest->y -= instr->dest->h;
                    break;
            }
        } 
    }

    SDL_RenderCopy(renderer, instr->texture, NULL, instr->dest);
}