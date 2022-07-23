// .\source\engine.c
enum SABRE_GameStatesEnum
{
    SABRE_UNINITIALIZED = 0,
    SABRE_RUNNING,
    SABRE_FINISHED
}SABRE_gameState = SABRE_UNINITIALIZED;

struct SABRE_CameraStruct
{
    struct SABRE_Vector2Struct prevPos;
    struct SABRE_Vector2Struct pos;
    struct SABRE_Vector2Struct dir;
    struct SABRE_Vector2Struct plane;
}SABRE_camera;

struct SABRE_KeybindStruct
{
    char forward, backward;
    char turnLeft, turnRight;
    char strafeLeft, strafeRight;
    char interact;
}SABRE_binds =
{
    KEY_w, KEY_s, // forward, backward
    KEY_a, KEY_d, // turn left, right
    KEY_q, KEY_e, // strafe left, right
    KEY_r         // interact
};

struct SABRE_KeyboardState
{
    char forward, backward;
    char turnLeft, turnRight;
    char strafeLeft, strafeRight;
    char interact;
}SABRE_keys;

struct SABRE_PlayerStruct
{
    float moveSpeed;
    float turnSpeed;
    struct SABRE_KeybindStruct keys;
    struct SABRE_CameraStruct camera;
}SABRE_player;

struct SABRE_SliceStruct
{
    short anim;
    short slice;
}SABRE_slice;

// GEMHP-end-of-declarations

// x = player
// 000     abc
// 0x0  => d e
// 000     fgh

#define SABRE_COLLISION_MASK_SIZE 7
#define SABRE_TOP_L      0x80 // a
#define SABRE_TOP_L_MASK 0xD0 // a && b && d
#define SABRE_TOP        0x40 // b
#define SABRE_TOP_MASK   0x40 // b
#define SABRE_TOP_R      0x20 // c
#define SABRE_TOP_R_MASK 0x68 // c && b && e
#define SABRE_LEFT       0x10 // d
#define SABRE_LEFT_MASK  0x10 // d
#define SABRE_RIGHT      0x08 // e
#define SABRE_RIGHT_MASK 0x08 // e
#define SABRE_LOW_L      0x04 // f
#define SABRE_LOW_L_MASK 0x16 // f && d && g
#define SABRE_LOW        0x02 // g
#define SABRE_LOW_MASK   0x02 // g
#define SABRE_LOW_R      0x01 // h
#define SABRE_LOW_R_MASK 0x0B // h && e && g

int SABRE_GetSurroundingWalls(float *px, float *py, int map[LEVEL_HEIGHT][LEVEL_WIDTH])
{
    int i, j, rows = 3, cols = 3, mid = 0, collisions = 0;

    for (j = 0; j < rows; j++)
    {
        for (i = 0; i < cols; i++)
        {
            if (j == 1 && i == 1) mid = 1;
            else
            {
                int row = (int)*py - 1 + j;
                int col = (int)*px - 1 + i;

                collisions += (map[row][col] > 0) << SABRE_COLLISION_MASK_SIZE - (j * cols + i - mid);
            }
        }
    }

    return collisions;
}

void SABRE_KeepDistance(float *x1, float *y1, float x2, float y2, float dist)
{
    float len = distance(*x1, *y1, x2, y2);
    float ang = degtorad(direction(x2, y2, *x1, *y1));

    if (len < dist)
    {
        // multiply the results by 1.001 to make the above condition evaluate
        // to false if the player hasn't moved since their position was last
        // modified by this function
        *x1 = x2 + cos(ang) * dist * 1.001f;
        *y1 = y2 + -sin(ang) * dist * 1.001f;
    }
}

void SABRE_UpdateKeyboardState()
{
    char *keys = GetKeyState();

    SABRE_keys.forward      = keys[SABRE_binds.forward];
    SABRE_keys.backward     = keys[SABRE_binds.backward];
    SABRE_keys.turnLeft     = keys[SABRE_binds.turnLeft];
    SABRE_keys.turnRight    = keys[SABRE_binds.turnRight];
    SABRE_keys.strafeLeft   = keys[SABRE_binds.strafeLeft];
    SABRE_keys.strafeRight  = keys[SABRE_binds.strafeRight];
    SABRE_keys.interact     = keys[SABRE_binds.interact];
}

void SABRE_Quit()
{
    if (SABRE_gameState != SABRE_FINISHED)
    {
        VisibilityState("SABRE_Screen", DISABLE);
        VisibilityState("SABRE_PlayerController", DISABLE);
        VisibilityState("SABRE_TextureActor", DISABLE);
        VisibilityState("SABRE_SpriteSlice", DISABLE);
        EventDisable("SABRE_Screen", EVENTALL);
        EventDisable("SABRE_PlayerController", EVENTALL);
        EventDisable("SABRE_TextureActor", EVENTALL);
        EventDisable("SABRE_SpriteSlice", EVENTALL);
        SABRE_gameState = SABRE_FINISHED;
    }
}


// .\source\jokuvaan.c


// .\source\misc-functionsss.c
int SABRE_StringEndsWith(const char *str, const char *str2)
{
    size_t len1 = strlen(str);
    size_t len2 = strlen(str2);

    if (len1 < len2)
    {
        return 0;
    }

    return !strcmp(&str[len1 - len2], str2);
}


// .\source\playercontroller-draw-actorie.c
struct SABRE_KeyboardState *keys = &SABRE_keys; // a pointer to the player key binds
struct SABRE_CameraStruct *camera = &SABRE_camera; // a pointer to the camera

struct SABRE_Vector2Struct oldDir   = camera->dir;
struct SABRE_Vector2Struct oldPlane = camera->plane;
float rotateSpeed = SABRE_player.turnSpeed;
float moveSpeed = SABRE_player.moveSpeed;

struct SABRE_Vector2Struct prev = camera->pos;
struct SABRE_Vector2Struct newPos = SABRE_CreateVector2(0.0f, 0.0f);
struct SABRE_Vector2Struct normalizedForwardDir;
struct SABRE_Vector2Struct normalizedRightDir;

SABRE_UpdateKeyboardState();

if (keys->turnLeft && !keys->turnRight)
{
    SABRE_RotateVector2InPlace(&camera->dir,    -rotateSpeed);
    SABRE_RotateVector2InPlace(&camera->plane,  -rotateSpeed);
}
else if (keys->turnRight && !keys->turnLeft)
{
    SABRE_RotateVector2InPlace(&camera->dir,    rotateSpeed);
    SABRE_RotateVector2InPlace(&camera->plane,  rotateSpeed);
}

normalizedForwardDir = SABRE_NormalizeVector2(camera->dir);
normalizedRightDir = SABRE_NormalizeVector2(camera->plane);

if (keys->forward && !keys->backward)
{
    SABRE_AddVector2InPlace(&newPos, SABRE_ScaleVector2(normalizedForwardDir, moveSpeed));
}
else if (keys->backward && !keys->forward)
{
    SABRE_AddVector2InPlace(&newPos, SABRE_ScaleVector2(normalizedForwardDir, -moveSpeed));
}

if (keys->strafeLeft && !keys->strafeRight)
{
    SABRE_AddVector2InPlace(&newPos, SABRE_ScaleVector2(normalizedRightDir, -moveSpeed));
}
else if (keys->strafeRight && !keys->strafeLeft)
{
    SABRE_AddVector2InPlace(&newPos, SABRE_ScaleVector2(normalizedRightDir, moveSpeed));
}

if (newPos.x != 0 || newPos.y != 0)
{
    SABRE_NormalizeVector2InPlace(&newPos);
    SABRE_ScaleVector2InPlace(&newPos, moveSpeed);

    {
        unsigned int i;
        float posX = newPos.x + camera->pos.x, posY = newPos.y + camera->pos.y;
        int coll = SABRE_GetSurroundingWalls(&posX, &posY, map);
        float radius = 0.4f;

        if ((coll & SABRE_TOP_L_MASK) == SABRE_TOP_L)
            SABRE_KeepDistance(&posX, &posY, (int)posX, (int)posY, radius);
        if ((coll & SABRE_TOP_MASK) == SABRE_TOP)
            SABRE_KeepDistance(&posX, &posY, posX, (int)posY, radius);
        if ((coll & SABRE_TOP_R_MASK) == SABRE_TOP_R)
            SABRE_KeepDistance(&posX, &posY, (int)posX + 1, (int)posY, radius);
        if ((coll & SABRE_LEFT_MASK) == SABRE_LEFT)
            SABRE_KeepDistance(&posX, &posY, (int)posX, posY, radius);
        if ((coll & SABRE_RIGHT_MASK) == SABRE_RIGHT)
            SABRE_KeepDistance(&posX, &posY, (int)posX +  1, posY, radius);
        if ((coll & SABRE_LOW_L_MASK) == SABRE_LOW_L)
            SABRE_KeepDistance(&posX, &posY, (int)posX, (int)posY + 1, radius);
        if ((coll & SABRE_LOW_MASK) == SABRE_LOW)
            SABRE_KeepDistance(&posX, &posY, posX, (int)posY + 1, radius);
        if ((coll & SABRE_LOW_R_MASK) == SABRE_LOW_R)
            SABRE_KeepDistance(&posX, &posY, (int)posX + 1, (int)posY + 1, radius);

        for (i = 0; i < SABRE_SPRITE_COUNT; i++)
        {
            SABRE_KeepDistance(&posX, &posY, sprites[i].pos.x, sprites[i].pos.y, sprites[i].radius);
        }

        camera->pos = SABRE_CreateVector2(posX, posY);
    }
}

if (prev.x != camera->pos.x || prev.y != camera->pos.y)
{
    camera->prevPos = prev;
}


// .\source\renderer.c
#ifndef SABRE_SPRITE_ACTOR
#define SABRE_SPRITE_ACTOR "SABRE_SpriteActor"
#endif

#ifndef SABRE_TEXTURE_ACTOR
#define SABRE_TEXTURE_ACTOR "SABRE_TextureActor"
#endif

enum SABRE_RenderObjectTypeEnum
{
    SABRE_TEXTURE_RO,
    SABRE_SPRITE_RO
};

typedef struct SABRE_RenderObjectStruct
{
    float sortValue;
    enum SABRE_RenderObjectTypeEnum objectType;

    float scale;
    int horizontalPosition;
    int horizontalScalingCompensation;
    struct SABRE_SliceStruct slice;

    struct SABRE_RenderObjectStruct *prev;
    struct SABRE_RenderObjectStruct *next;
}SABRE_RenderObject;

SABRE_RenderObject *mapROs[LEVEL_HEIGHT][LEVEL_WIDTH];

struct SABRE_RenderObjectListManagerStruct
{
    SABRE_RenderObject *head;
    SABRE_RenderObject *curr;
    SABRE_RenderObject *reusableCache;
}SABRE_ROListManager;

#if DEBUG
int allocations = 0;
int traversals = 0;
int singleSliceTraversals = 0;
int maxTraversals = 0;
size_t allocatedMemory = 0;
#endif
// GEMHP-end-of-declarations
void SABRE_FreeRenderObjectList()
{
    SABRE_RenderObject *iterator = NULL;
    SABRE_RenderObject *next = NULL;

    for (iterator = SABRE_ROListManager.head; iterator != NULL; iterator = next)
    {
        next = iterator->next;
        free(iterator);

#if DEBUG
        allocations--;
        allocatedMemory -= sizeof *iterator;
#endif
    }

    SABRE_ROListManager.head = NULL;
    SABRE_ROListManager.curr = NULL;
    SABRE_ROListManager.reusableCache = NULL;
}

SABRE_RenderObject *SABRE_GetLastROInList(SABRE_RenderObject *list)
{
    SABRE_RenderObject *iterator = NULL;

    for (iterator = list; iterator != NULL; iterator = iterator->next)
    {
        if (!iterator->next)
        {
            return iterator;
        }
    }

    return NULL;
}

SABRE_RenderObject *SABRE_ConcatenateROList(SABRE_RenderObject *dest, SABRE_RenderObject *src)
{
    SABRE_RenderObject *tail = SABRE_GetLastROInList(dest);
    SABRE_RenderObject *result = NULL;

    if (tail)
    {
        tail->next = src;
        result = dest;
    }
    else
    {
        result = src;
    }

    return result;
}

void SABRE_InitializeFrame()
{
    int i, j;

    for (j = 0; j < LEVEL_HEIGHT; j++)
    {
        for (i = 0; i < LEVEL_WIDTH; i++)
        {
            mapROs[j][i] = NULL;
        }
    }

#if DEBUG
    traversals = 0;
    maxTraversals = 0;
#endif

    SABRE_ROListManager.reusableCache = SABRE_ConcatenateROList(SABRE_ROListManager.reusableCache, SABRE_ROListManager.head);
    SABRE_ROListManager.head = NULL;
    SABRE_ROListManager.curr = NULL;
}

int SABRE_InsertRO(SABRE_RenderObject *object)
{
    SABRE_RenderObject *iterator = NULL;
    SABRE_RenderObject *prev = NULL;
    SABRE_RenderObject *next = NULL;

    if (!object)
    {
        return 1;
    }

    if (!SABRE_ROListManager.head || !SABRE_ROListManager.curr)
    {
        SABRE_ROListManager.head = object;
        SABRE_ROListManager.curr = object;
        return 0;
    }

    iterator = SABRE_ROListManager.curr;

#if DEBUG
    singleSliceTraversals = 0;
#endif

    if (object->sortValue <= iterator->sortValue)
    {
        while (iterator && object->sortValue <= iterator->sortValue)
        {
            prev = iterator;
            iterator = iterator->next;
#if DEBUG
            traversals++;
            singleSliceTraversals++;
#endif
        }

        if (iterator)
        {
            object->prev = iterator->prev;
            object->next = iterator;
            if (iterator->prev)
            {
                iterator->prev->next = object;
            }
            iterator->prev = object;

        }
        else
        {
            object->prev = prev;
            object->next = NULL;
            prev->next = object;
        }
        SABRE_ROListManager.curr = object;
    }
    else
    {
        while (iterator && object->sortValue > iterator->sortValue)
        {
            next = iterator;
            iterator = iterator->prev;
#if DEBUG
            traversals++;
            singleSliceTraversals++;
#endif
        }

        if (iterator)
        {
            object->prev = iterator;
            object->next = iterator->next;
            if (iterator->next)
            {
                iterator->next->prev = object;
            }
            iterator->next = object;
        }
        else
        {
            object->prev = NULL;
            object->next = next;
            next->prev = object;
            SABRE_ROListManager.head = object;
        }
        SABRE_ROListManager.curr = object;
    }

#if DEBUG
    if (singleSliceTraversals > maxTraversals) maxTraversals = singleSliceTraversals;
#endif

    return 0;
}

SABRE_RenderObject *SABRE_GetNextUnusedRO()
{
    if (SABRE_ROListManager.reusableCache)
    {
        SABRE_RenderObject *new = SABRE_ROListManager.reusableCache;
        SABRE_ROListManager.reusableCache = SABRE_ROListManager.reusableCache->next;
        return new;
    }
    else
    {
        SABRE_RenderObject *new = malloc(sizeof *new);

        if (!new)
        {
            DEBUG_MSG_FROM("Memory allocation failed!", "SABRE_GetNextUnusedRO");
            return NULL;
        }

#if DEBUG
    allocations++;
    allocatedMemory += sizeof *new;
#endif

        return new;
    }
}

SABRE_RenderObject *SABRE_AddTextureRO(float sortValue, float scale, int horizontalPosition, int compensation, struct SABRE_SliceStruct slice)
{
    int err = 0;
    SABRE_RenderObject *new = SABRE_GetNextUnusedRO();

    if (!new)
    {
        return NULL;
    }

    new->sortValue = sortValue;
    new->objectType = SABRE_TEXTURE_RO;
    new->scale = scale;
    new->horizontalPosition = horizontalPosition;
    new->horizontalScalingCompensation = compensation;
    new->slice = slice;
    new->prev = NULL;
    new->next = NULL;

    err = SABRE_InsertRO(new);

    return new;
}

SABRE_RenderObject *SABRE_AddSpriteRO(float sortValue, float scale, int horizontalPosition, struct SABRE_SliceStruct slice)
{
    int err = 0;
    SABRE_RenderObject *new = SABRE_GetNextUnusedRO();

    if (!new)
    {
        return NULL;
    }

    new->sortValue = sortValue;
    new->objectType = SABRE_SPRITE_RO;
    new->scale = scale;
    new->horizontalPosition = horizontalPosition;
    new->horizontalScalingCompensation = 0;
    new->slice = slice;
    new->prev = NULL;
    new->next = NULL;

    err = SABRE_InsertRO(new);

    return new;
}

void SABRE_PrintROList()
{
    int counter = 0;
    char temp[256];
    struct SABRE_RenderObjectStruct *iterator = NULL;

    sprintf(temp, "head: %f, curr: %f", SABRE_ROListManager.head->sortValue, SABRE_ROListManager.curr->sortValue);
    DEBUG_MSG(temp);

    for (iterator = SABRE_ROListManager.head; iterator != NULL; iterator = iterator->next)
    {
        sprintf(temp, "frame: %3d render object %3d: [sortValue: %f, scale: %f, hpos: %d, compensation: %d, slice: [anim: %d, slice: %d]]",
            frame, counter++, iterator->sortValue, iterator->scale, iterator->horizontalPosition, iterator->horizontalScalingCompensation,
            iterator->slice.anim, iterator->slice.slice);
        DEBUG_MSG(temp);
    }
}

void SABRE_RenderObjects()
{
    int horizontalPosition = 0;
    float verticalPosition = height * 0.5f;
    SABRE_RenderObject *iterator = NULL;
    float verticalResolutionFactor = screenHeight / 480.0f;
    const float horizontalCompensationThreshold = 0.0315f; // threshold for growing the compensation

    for (iterator = SABRE_ROListManager.head; iterator != NULL; iterator = iterator->next)
    {
        if (iterator->objectType == SABRE_TEXTURE_RO)
        {
            SABRE_slice.anim = iterator->slice.anim;
            SABRE_slice.slice = iterator->slice.slice;
            SendActivationEvent(SABRE_TEXTURE_ACTOR);
            draw_from(SABRE_TEXTURE_ACTOR, iterator->horizontalPosition + iterator->horizontalScalingCompensation, verticalPosition, iterator->scale);
        }
        else if (iterator->objectType == SABRE_SPRITE_RO)
        {
            SABRE_slice.anim = iterator->slice.anim;
            SABRE_slice.slice = iterator->slice.slice;
            SendActivationEvent(SABRE_SPRITE_ACTOR);
            draw_from(SABRE_SPRITE_ACTOR, iterator->horizontalPosition, verticalPosition + (((480.0f * iterator->scale) - ((float)iterator->scale * (float)getclone("SABRE_SpriteActor.0")->height))*0.5f) * verticalResolutionFactor, iterator->scale * verticalResolutionFactor);
        }
    }
}


// .\source\screen-draw-actor.c
short slice; // slice of screen to be drawn
short wallHit; // was a wall hit or not
short hitSide; // which side was hit: x-side (0) or y-side (1)

float cameraX; // x-coordinate on the camera plane (-1.0 - 1.0)
float wallHitX; // the horizontal position where the ray hit the wall (0.0-1.0)
float wallSliceHeight; // height of the wall slice to draw
float perpWallDist; // perpendicular distance from the wall (ray length)

float rayPosX, rayPosY; // ray position
float rayDirX, rayDirY; // ray direction
short rayMapX, rayMapY; // coordinates of the square the ray is in
short rayStepX, rayStepY; // ray step direction
float sideDistX, sideDistY; // distance from ray position to next x- or y-side TODO: better explanation
float deltaDistX, deltaDistY; // distance ray has to travel from one side to another

float scale;
short horizontalScalingCompensation; // amount of pixels to shift the drawing position to right
                                     // to compensate for the bigger width resulting from scaling
const float horizontalCompensationThreshold = 0.0315f; // threshold for growing the compensation

struct SABRE_TextureStruct *texture = NULL;
struct SABRE_CameraStruct *camera = &SABRE_camera; // a pointer to the camera

unsigned int sprite; // index of the sprite to render
float spriteX, spriteY;
float invDet;
float transformX, transformY;
float spriteScreenX;

char solidWallHit = 0;

// only the 1st clone (cloneindex 0) will execute this code, as the other ones are just going
// to inherit everything drawn on the first clone, due to how cloned canvases work in GE
if (!cloneindex && SABRE_gameState == SABRE_RUNNING)
{
    erase(0, 0, 0, 1);
    SABRE_InitializeFrame();

    drawCalls = 0;

    invDet = 1.0f / (float)(camera->plane.x * camera->dir.y - camera->dir.x * camera->plane.y);

    for (sprite = 0; sprite < SABRE_SPRITE_COUNT; sprite++)
    {
        spriteX = sprites[sprite].pos.x - camera->pos.x;
        spriteY = sprites[sprite].pos.y - camera->pos.y;

        transformX = invDet * (camera->dir.y * spriteX - camera->dir.x * spriteY);
        transformY = invDet * (-camera->plane.y * spriteX + camera->plane.x * spriteY);

        if (transformY < 0.05f)
        {
            transformY = -1;
        }

        spriteScreenX = (screenWidth / 2.0f) * (1 + transformX / transformY);

        if (transformY > 0)
        {
            SABRE_slice.anim = sprites[sprite].sprite;
            SABRE_slice.slice = 0;
            SABRE_AddSpriteRO(transformY, 1.0f / transformY, spriteScreenX, SABRE_slice);
        }
    }

    for (slice = 0; slice < screenWidth; slice++)
    {
        // calculate the position and direction of the ray
        cameraX = 2.0f * (float)slice / (float)screenWidth - 1; // x on the camera plane
        rayPosX = camera->pos.x; // set the begin position of the ray to the player's position
        rayPosY = camera->pos.y;
        rayDirX = camera->dir.x + camera->plane.x * cameraX; // set the direction of the ray
        rayDirY = camera->dir.y + camera->plane.y * cameraX;

        // set the square the ray starts from
        rayMapX = (short)rayPosX;
        rayMapY = (short)rayPosY;

        // distance ray has to travel from one side to another
        deltaDistX = (rayDirX == 0) ? 1e30 : abs(1 / rayDirX);
        deltaDistY = (rayDirY == 0) ? 1e30 : abs(1 / rayDirY);

        wallHit = 0;
        hitSide = 0;

        // calculate the step and the initial sideDistX and sideDistY
        if (rayDirX < 0) // if the ray is moving left
        {
            rayStepX = -1;
            sideDistX = (rayPosX - rayMapX) * deltaDistX;
        }
        else // if the ray is moving right insted
        {
            rayStepX = 1;
            sideDistX = (rayMapX + 1 - rayPosX) * deltaDistX;
        }
        if (rayDirY < 0) // if the ray is moving up
        {
            rayStepY = -1;
            sideDistY = (rayPosY - rayMapY) * deltaDistY;
        }
        else // if the ray is moving down instead
        {
            rayStepY = 1;
            sideDistY = (rayMapY + 1 - rayPosY) * deltaDistY;
        }

        solidWallHit = 0;

        while (!solidWallHit)
        {
            // perform Digital Differential Analysis (DDA) for finding the wall
            while (!wallHit) // loop until a wall has been hit
            {
                // step to next map square in x- or y-direction, depending on which one is closer
                if (sideDistX < sideDistY)
                {
                    sideDistX += deltaDistX;
                    rayMapX += rayStepX;
                    hitSide = 0;
                }
                else
                {
                    sideDistY += deltaDistY;
                    rayMapY += rayStepY;
                    hitSide = 1;
                }

                // check if a wall has been hit
                if (map[rayMapY][rayMapX] > 0) wallHit = 1;
            }

            // calculate the perpendicular distance between the wall and the camera plane
            if (!hitSide)
                perpWallDist = (sideDistX - deltaDistX);
            else
                perpWallDist = (sideDistY - deltaDistY);

            // calculate the height of the current line of the wall to be drawn
            wallSliceHeight = (float)screenHeight / (float)perpWallDist;

            // calculate the right texture to use
            SABRE_slice.anim = map[rayMapY][rayMapX] - 1;
            texture = &SABRE_GET_TEXTURE(&SABRE_textureStore, SABRE_slice.anim);

            // calculate where the wall was hit
            if (hitSide)
                wallHitX = rayPosX + ((rayMapY - rayPosY + (1 - rayStepY) / 2.0f) / rayDirY) * rayDirX;
            else
                wallHitX = rayPosY + ((rayMapX - rayPosX + (1 - rayStepX) / 2.0f) / rayDirX) * rayDirY;

            // get only the decimal part, which represents the hit position along the horizontal
            // axis of the texture
            wallHitX -= (short)wallHitX;

            // calculate which vertical slice from the texture has to be drawn
            SABRE_slice.slice = (short)(wallHitX * texture->width);

            // prevent textures from being drawn as mirror images
            if (!hitSide && rayDirX < 0) SABRE_slice.slice = texture->width - SABRE_slice.slice - 1;
            if (hitSide && rayDirY > 0) SABRE_slice.slice = texture->width - SABRE_slice.slice - 1;

            scale = wallSliceHeight / (float)texture->height;
            horizontalScalingCompensation = (short)floor(scale - horizontalCompensationThreshold) + 1;

            // If a part of this wall has already been drawn this frame, set the render object list manager's
            // curr pointer to point to that render object in the list, as the next slice will be somewhere
            // very near to that in depth
            if (mapROs[rayMapY][rayMapX])
            {
                SABRE_ROListManager.curr = mapROs[rayMapY][rayMapX];
            }

            SABRE_AddTextureRO(perpWallDist, scale, slice, horizontalScalingCompensation, SABRE_slice);

            // Set the last used render object pointer for this wall
            mapROs[rayMapY][rayMapX] = SABRE_ROListManager.curr;

            if (!texture->isWindow)
            {
                solidWallHit = 1;
            }

            wallHit = 0;
            drawCalls++;
        }

        slice += (short)max(floor(scale) - 1, 0) + horizontalScalingCompensation * (scale > 1.0f - (horizontalCompensationThreshold - 0.0001f));
    }

    SABRE_RenderObjects();
}


// .\source\sprite.c
#ifndef SABRE_SPRITE_ACTOR
#define SABRE_SPRITE_ACTOR "SABRE_SpriteActor"
#endif

struct SABRE_SpriteStruct
{
    struct SABRE_Vector2Struct pos;
    float radius;
    unsigned int sprite;
};

#define SABRE_SPRITE_COUNT 11

struct SABRE_SpriteStruct sprites[SABRE_SPRITE_COUNT] =
{
    { { 8.5f, 1.5f }, 0.4f, 1 },
    { { 7.5f, 1.5f }, 0.4f, 1 },
    { { 6.5f, 1.5f }, 0.4f, 2 },
    { { 8.5f, 2.5f }, 0.4f, 1 },
    { { 7.5f, 2.5f }, 0.4f, 0 },
    { { 6.5f, 2.5f }, 0.4f, 1 },
    { { 6.5f, 3.5f }, 0.4f, 3 },
    { { 7.0f, 3.5f }, 0.4f, 3 },
    { { 7.5f, 3.5f }, 0.4f, 3 },
    { { 8.0f, 3.5f }, 0.4f, 3 },
    { { 8.5f, 3.5f }, 0.4f, 3 }
};




// .\source\texture.c
#ifndef SABRE_TEXTURE_ACTOR
#define SABRE_TEXTURE_ACTOR "SABRE_TextureActor"
#endif

struct SABRE_TextureStruct
{
    int width;
    int height;
    short slices;
    char isWindow;
    char name[256];
};

#define SABRE_DATA_STORE_DOUBLING_LIMIT 128
#define SABRE_DATA_STORE_GROW_AMOUNT 64 // SABRE_DATA_STORE_DOUBLING_LIMIT / 2

#define SABRE_GET_TEXTURE(DATA_STORE, INDEX) ((struct SABRE_TextureStruct *)(DATA_STORE)->elems)[INDEX]

struct SABRE_DataStoreStruct
{
    size_t capacity; // the maximum amount of elements the store can hold at the moment
    size_t count; // the amount of elements the store actually holds at the moment
    size_t elemSize; // the size of a single element in the store
    void *elems; // pointer to the elements
    void (*addFunc)(struct SABRE_DataStoreStruct*, void*);
}SABRE_textureStore;

int SABRE_AutoAddTextures();
int SABRE_AddTexture(const char textureName[256]);
int SABRE_CalculateTextureWidth(struct SABRE_TextureStruct *texture);
int SABRE_CalculateTextureHeight(struct SABRE_TextureStruct *texture);

void SABRE_AddTextureToDataStore(struct SABRE_DataStoreStruct *dataStore, void *texture);

void SABRE_SetDataStoreAddFunc(struct SABRE_DataStoreStruct *dataStore, void (*addDataFunc)(struct SABRE_DataStoreStruct*, void*));
int SABRE_InitDataStore(struct SABRE_DataStoreStruct *dataStore, size_t elemSize);
int SABRE_GrowDataStore(struct SABRE_DataStoreStruct *dataStore);
int SABRE_PrepareDataStore(struct SABRE_DataStoreStruct *dataStore);
int SABRE_AddToDataStore(struct SABRE_DataStoreStruct *dataStore, void *elem);
void SABRE_FreeDataStore(struct SABRE_DataStoreStruct *dataStore);
// GEMHP-end-of-declarations
// only works for non-animated textures
int SABRE_AutoAddTextures()
{
    int i = 0;
    int err = 0;
    char animName[256];

    strcpy(animName, getAnimName(i));
    SABRE_SetDataStoreAddFunc(&SABRE_textureStore, SABRE_AddTextureToDataStore);
    SABRE_textureStore.elemSize = sizeof(struct SABRE_TextureStruct);

    while (strcmp(animName, ""))
    {
        err = SABRE_AddTexture(animName);

        if (err) return err;

        #if DEBUG
        {
            char temp[256];
            sprintf(temp, "Added texture: [%d %s]", i, animName);
            DEBUG_MSG(temp);
        }
        #endif

        strcpy(animName, getAnimName(++i));
    }

    return 0;
}

int SABRE_AddTexture(const char textureName[256])
{
    struct SABRE_TextureStruct newTexture;

    strcpy(newTexture.name, textureName);
    newTexture.width = SABRE_CalculateTextureWidth(&newTexture);
    newTexture.height = SABRE_CalculateTextureHeight(&newTexture);
    newTexture.isWindow = SABRE_StringEndsWith(newTexture.name, "-window");

    return SABRE_AddToDataStore(&SABRE_textureStore, &newTexture);
}

int SABRE_CalculateTextureWidth(struct SABRE_TextureStruct *texture)
{
    // TODO: make a check for if the animation actually exists, use getAnimIndex(), if -1, doesn't exist
    ChangeAnimation(SABRE_TEXTURE_ACTOR, texture->name, STOPPED);
    return getclone(SABRE_TEXTURE_ACTOR)->nframes;
}

int SABRE_CalculateTextureHeight(struct SABRE_TextureStruct *texture)
{
    int i;
    int textureHeight = 0;
    Actor *textureActor = getclone(SABRE_TEXTURE_ACTOR);

    // TODO: make a check for if the animation actually exists, use getAnimIndex(), if -1, doesn't exist
    ChangeAnimation(SABRE_TEXTURE_ACTOR, texture->name, STOPPED);

    for (i = 0; i < textureActor->nframes; i++)
    {
        textureActor = getclone(SABRE_TEXTURE_ACTOR); // this updates the width and height values

        if (textureActor->height > textureHeight)
        {
            textureHeight = textureActor->height;
        }

        textureActor->animpos++;
        // Send activation event to apply the animpos advancement during this frame already.
        // The normal behavior of Game Editor is to update the animpos of an actor in the next
        // frame. This same trick is used for changing the TextureSlice's animpos multiple
        // times per frame when drawing the game view. Notice, however, that if you were to
        // inspect the actor during a frame where this trick is used you still wouldn't see
        // the animpos changing more than once during a single frame. This is because Game Editor
        // only draws the actor on screen once per frame. But behind the scenes the animpos
        // still changes multiple times per frame, affecting the actor's dimensions as well as
        // its appearance if drawn using draw_from().
        SendActivationEvent(SABRE_TEXTURE_ACTOR);
    }

    return textureHeight;
}

void SABRE_AddTextureToDataStore(struct SABRE_DataStoreStruct *dataStore, void *texture)
{
    SABRE_GET_TEXTURE(dataStore, dataStore->count) = (*(struct SABRE_TextureStruct *)texture);
}

void SABRE_SetDataStoreAddFunc(struct SABRE_DataStoreStruct *dataStore, void (*addDataFunc)(struct SABRE_DataStoreStruct*, void*))
{
    dataStore->addFunc = addDataFunc;
}

int SABRE_InitDataStore(struct SABRE_DataStoreStruct *dataStore, size_t elemSize)
{
    dataStore->capacity = 16;
    dataStore->count = 0;
    dataStore->elemSize = elemSize;
    dataStore->elems = calloc(dataStore->capacity, dataStore->elemSize);

    if (!dataStore->elems)
    {
        DEBUG_MSG_FROM("Memory allocation failed!", "SABRE_InitDataStore");
        return 1;
    }

    return 0;
}

int SABRE_GrowDataStore(struct SABRE_DataStoreStruct *dataStore)
{
    void *newElems = NULL;

    // double the data store size or grow it by SABRE_DATA_STORE_GROW_AMOUNT
    if (dataStore->capacity < SABRE_DATA_STORE_DOUBLING_LIMIT) dataStore->capacity *= 2;
    else dataStore->capacity += SABRE_DATA_STORE_GROW_AMOUNT;

    newElems = realloc(dataStore->elems, dataStore->capacity);

    if (!newElems)
    {
        DEBUG_MSG_FROM("Memory allocation failed!", "SABRE_GrowDataStore");
        return 1;
    }

    dataStore->elems = newElems;

    return 0;
}

int SABRE_PrepareDataStore(struct SABRE_DataStoreStruct *dataStore)
{
    // the data store has not been initialized, initialize it and make sure no errors occurred
    if (!dataStore->capacity && SABRE_InitDataStore(dataStore, dataStore->elemSize) != 0)
    {
        DEBUG_MSG_FROM("Unable to initialize data store.", "SABRE_PrepareDataStore");
        return 1;
    }
    // the data store is full, grow it and make sure no errors occurred
    else if (dataStore->count == dataStore->capacity && SABRE_GrowDataStore(dataStore) != 0)
    {
        DEBUG_MSG_FROM("Unable to grow data store.", "SABRE_PrepareDataStore");
        return 2;
    }
    // otherwise no-op

    return 0;
}

int SABRE_AddToDataStore(struct SABRE_DataStoreStruct *dataStore, void *elem)
{
    int err = 0;

    err = SABRE_PrepareDataStore(dataStore);

    if (err != 0) return err;

    dataStore->addFunc(dataStore, elem);
    dataStore->count++; // new element has been added, increment count

    return 0;
}

void SABRE_FreeDataStore(struct SABRE_DataStoreStruct *dataStore)
{
    if (dataStore->elems)
    {
        free(dataStore->elems);
        dataStore->capacity = 0;
        dataStore->count = 0;
        dataStore->elemSize = 0;
        dataStore->elems = NULL;
    }
}


// .\source\vector2.c
struct SABRE_Vector2Struct
{
    float x, y;
};
// GEMHP-end-of-declarations
struct SABRE_Vector2Struct SABRE_CreateVector2(float x, float y)
{
    struct SABRE_Vector2Struct new;
    new.x = x;
    new.y = y;
    return new;
}

struct SABRE_Vector2Struct SABRE_ScaleVector2(struct SABRE_Vector2Struct vec, float scale)
{
    struct SABRE_Vector2Struct new;
    new.x = vec.x * scale;
    new.y = vec.y * scale;
    return new;
}

void SABRE_ScaleVector2InPlace(struct SABRE_Vector2Struct *vec, float scale)
{
    vec->x *= scale;
    vec->y *= scale;
}

struct SABRE_Vector2Struct SABRE_RotateVector2(struct SABRE_Vector2Struct vec, float rotation)
{
    struct SABRE_Vector2Struct new;
    new.x = vec.x * cos(rotation) - vec.y * sin(rotation);
    new.y = vec.x * sin(rotation) + vec.y * cos(rotation);
    return new;
}

void SABRE_RotateVector2InPlace(struct SABRE_Vector2Struct *vec, float rotation)
{
    float oldX = vec->x;
    vec->x = vec->x * cos(rotation) - vec->y * sin(rotation);
    vec->y = oldX   * sin(rotation) + vec->y * cos(rotation);
}

struct SABRE_Vector2Struct SABRE_AddVector2(struct SABRE_Vector2Struct a, struct SABRE_Vector2Struct b)
{
    struct SABRE_Vector2Struct new;
    new.x = a.x + b.x;
    new.y = a.y + b.y;
    return new;
}

void SABRE_AddVector2InPlace(struct SABRE_Vector2Struct *a, struct SABRE_Vector2Struct b)
{
    a->x += b.x;
    a->y += b.y;
}

float SABRE_DotProductVector2(struct SABRE_Vector2Struct a, struct SABRE_Vector2Struct b)
{
    return a.x * b.x + a.y * b.y;
}

float SABRE_MagnitudeVector2(struct SABRE_Vector2Struct a)
{
    return sqrt(SABRE_DotProductVector2(a, a));
}

struct SABRE_Vector2Struct SABRE_NormalizeVector2(struct SABRE_Vector2Struct vec)
{
    float magnitude = SABRE_MagnitudeVector2(vec);

    if (abs(magnitude) <= 0.0001f)
    {
        DEBUG_MSG_FROM("Unable to normalize: vector magnitude was 0.", "SABRE_NormalizeVector2");
        return SABRE_CreateVector2(0.0f, 0.0f);
    }

    return SABRE_ScaleVector2(vec, 1.0f / magnitude);
}

void SABRE_NormalizeVector2InPlace(struct SABRE_Vector2Struct *vec)
{
    float magnitude = SABRE_MagnitudeVector2(*vec);

    if (abs(magnitude) <= 0.0001f)
    {
        DEBUG_MSG_FROM("Unable to normalize: vector magnitude was 0.", "SABRE_NormalizeVector2InPlace");
        vec->x = 0.0f;
        vec->y = 0.0f;
        return;
    }

    SABRE_ScaleVector2InPlace(vec, 1.0f / magnitude);
}


