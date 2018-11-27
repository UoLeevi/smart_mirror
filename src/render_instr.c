#include "render_instr.h"

void render_instr_set(
    render_instr *instr,
    SDL_Surface *surface,
    uo_relpoint reldest,
    int w,
    int h)
{
    instr->surface = surface;
    instr->reldest = reldest;
    instr->w = w;
    instr->h = h;
    instr->flags |= 0x7;
}

void render_instr_set_s(
    render_instr *instr,
    SDL_Surface *surface)
{
    instr->surface = surface;
    instr->flags |= instr->w ? 0x1 : 0x7;
}

void render_instr_set_swh(
    render_instr *instr,
    SDL_Surface *surface,
    int w,
    int h)
{
    instr->surface = surface;
    instr->w = w;
    instr->h = h;
    instr->flags |= 0x7;
}

void render_instr_set_xy(
    render_instr *instr,
    uo_relpoint reldest)
{
    instr->reldest = reldest;
    instr->flags |= 0x2;
}

bool render_instr_is_render_required(
    render_instr *instr)
{
    return instr->flags & 0x80;
}

void render_instr_reposition(
    render_instr *instr)
{
    instr->flags |= 0x2;
}

void render_instr_update(
    render_instr *instr)
{
    instr->flags |= 0x80;
}

void render_instr_render(
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

            if (instr->w)
            {
                instr->dest.w = instr->w;
                instr->dest.h = instr->h;
            }
            else if (instr->texture)
                SDL_QueryTexture(instr->texture, NULL, NULL, &instr->dest.w, &instr->dest.h);
        }

        if (instr->flags & 0x2)
        {
            instr->flags &= ~0x2;

            int w, h;
            SDL_GetRendererOutputSize(renderer, &w, &h);

            int x = instr->reldest.x.pct_screen * w / 100;
            int y = instr->reldest.y.pct_screen * h / 100;

            x += instr->reldest.x.pct_self * instr->dest.w / 100;
            y += instr->reldest.y.pct_self * instr->dest.h / 100;

            x += instr->reldest.x.px;
            y += instr->reldest.y.px;

            instr->dest.x = x;
            instr->dest.y = y;
        } 
    }

    if (instr->texture)
        SDL_RenderCopy(renderer, instr->texture, NULL, &instr->dest);
}