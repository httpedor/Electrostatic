#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <stdio.h>
#include <iostream>
#include <stdlib.h>

#define MAX_CHARGES 0xFF
#define MAX_ARROWS 2048
#define COULOMB_CONSTANT 0.01

template <typename T> int sgn(T val) {
    return (T(0) <= val) - (val < T(0));
}

struct Charge{
    int id;
    bool active;
    bool simulated;
    int charge;
    Vector2 position;
    Vector2 velocity;
};

struct FieldArrow{
    Vector2 position;
    Vector2 direction;
    float magnitude;
};

static Charge charges[MAX_CHARGES];
static FieldArrow fieldArrows[MAX_ARROWS];

float biggestMagnitude = 1;
bool isPaused = true;

int SimRemoveCharge(int id){
    for(int i = 0; i < MAX_CHARGES; i++){
        if(charges[i].id == id){
            charges[i].active = false;
            return i;
        }
    }
    return -1;
}

Charge* SimGetCharge(int id){
    return &charges[id];
}

int SimAddCharge(Vector2 position, float charge){
    for(int i = 0; i < MAX_CHARGES; i++){
        if(!charges[i].active){
            charges[i].id = i;
            charges[i].active = true;
            charges[i].simulated = true;
            charges[i].position = position;
            charges[i].velocity = Vector2Zero();
            charges[i].charge = charge;
            if (charge == 0)
                charges[i].simulated = true;
            return i;
        }
    }
    return -1;
}

void SimClear(){
    for(int i = 0; i < MAX_CHARGES; i++){
        charges[i].active = false;
    }
}

void SimInit(){
    for(int i = 0; i < MAX_CHARGES; i++){
        charges[i].id = i;
        charges[i].active = false;
    }
}

void SimStep(){
    float biggest = 0;
    for (int i = 0; i < MAX_ARROWS; i++)
    {
        FieldArrow& arrow = fieldArrows[i];
        if (!CheckCollisionPointRec(arrow.position, {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()}))
        {
            continue;
        }
        Vector2 acceleration = Vector2Zero();
        for (int i = 0; i < MAX_CHARGES; i++)
        {
            Charge& charge = charges[i];
            if (!charge.active)
                continue;

            Vector2 direction = Vector2Subtract(charge.position, arrow.position);
            float distance = Vector2Length(direction);
            float chargeOther = abs(charge.charge);
            int signOther = sgn(charge.charge);
            float mag = COULOMB_CONSTANT * (chargeOther / (distance * distance));
            if (signOther == 1)
                mag = -mag;

            acceleration = Vector2Add(acceleration, Vector2Scale(direction, mag));
        }

        arrow.direction = Vector2Normalize(acceleration);
        arrow.magnitude = abs(Vector2Length(acceleration));
        if (arrow.magnitude > biggest){
            biggest = arrow.magnitude;
        }
    }
    biggestMagnitude = biggest;
    if (isPaused){
        return;
    }
    for(int i = 0; i < MAX_CHARGES; i++){
        Charge& current = charges[i];

        if(!current.active){
            continue;
        }
        Vector2 acceleration = Vector2Zero();
        for(int j = 0; j < MAX_CHARGES; j++){
            Charge& target = charges[j];
            if(!target.active || i == j || !current.simulated){
                continue;
            }
            Vector2 direction = Vector2Subtract(target.position, current.position);
            float distance = Vector2Length(direction);
            float chargeCurrent = abs(current.charge);
            int signCurrent = sgn(current.charge);
            float chargeOther = abs(target.charge);
            int signOther = sgn(target.charge);
            float force;
            if (chargeCurrent == 0){
                force = COULOMB_CONSTANT * (chargeOther / (distance * distance));
            }
            else if (chargeOther == 0){
                force = COULOMB_CONSTANT * (chargeCurrent / (distance * distance));
            }
            else{
                force = COULOMB_CONSTANT * (chargeCurrent * chargeOther / (distance * distance));
            }
            if (signCurrent == signOther){
                force = -force;
            }
            acceleration = Vector2Add(acceleration, Vector2Scale(direction, force));

            direction = Vector2Subtract(Vector2Add(current.position, Vector2Add(current.velocity, acceleration)), target.position);
            distance = Vector2Length(direction);
            if (distance < 20){
                current.velocity = Vector2Zero();
                float distanceToClosestPoint = Vector2Length(Vector2Subtract(Vector2MoveTowards(current.position, target.position, 10), Vector2MoveTowards(target.position, current.position, 10)));
                current.position = Vector2Add(current.position, Vector2Scale(direction, distanceToClosestPoint));
                target.velocity = Vector2Zero();
            }
        }
        charges[i].velocity = Vector2Add(charges[i].velocity, acceleration);
        charges[i].position = Vector2Add(charges[i].position, charges[i].velocity);
    }
}

void SimResize(Vector2 oldSize, Vector2 newSize){
    int arrowSpacing = 30;
    int arrowRadius = 35;
    int arrowsInRow = (int)ceil(newSize.x / (arrowSpacing + arrowRadius));

    for (int i = 0; i < MAX_ARROWS; i++){
        float x = (i % arrowsInRow) * (arrowSpacing + arrowRadius);
        float y = (i / arrowsInRow) * (arrowSpacing + arrowRadius);
        fieldArrows[i].position = {x, y};
        fieldArrows[i].direction = Vector2Zero();
        fieldArrows[i].magnitude = 0;
    }
}