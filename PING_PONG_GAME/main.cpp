#include <iostream>
#include <raylib.h>

using namespace std;

Color Green = Color{38,185,154,255};
Color Dark_Green = Color{20,160,133,255};
Color Light_Green = Color{129,204,184,255};
Color Yellow = Color{243,213,91,255};

int player_score = 0;
int cpu_score = 0;

#define TRAIL_LENGTH 6  // Length of wind trail

class Ball {
public:
    float x, y;
    int speed_x, speed_y;
    int radius;
    Color color;
    Vector2 trail[TRAIL_LENGTH];

    Ball() {
        color = Yellow;
    }

    void Draw() {
        // Draw the wind trail effect
        for (int i = TRAIL_LENGTH - 1; i > 0; i--) {
            Color trailColor = Fade(color, (float)(i) / TRAIL_LENGTH);
            DrawCircle(trail[i].x, trail[i].y, radius * (1.0f - i * 0.2f), trailColor);
        }
        DrawCircle(x, y, radius, color);
    }

    void Update(Sound &missSound, Sound &hitSound) {
        // Shift trail positions
        for (int i = TRAIL_LENGTH - 1; i > 0; i--) {
            trail[i] = trail[i - 1];
        }
        trail[0] = {x, y}; // Store current position

        x += speed_x;
        y += speed_y;

        if (y + radius >= GetScreenHeight() || y - radius <= 0) {
            speed_y *= -1;
        }

        if (x + radius >= GetScreenWidth()) {
            cpu_score++;
            PlaySound(missSound);
            CheckWinner();
        }
        if (x - radius <= 0) {
            player_score++;
            PlaySound(missSound);
            CheckWinner();
        }
    }

    void CheckWinner() {
        if (cpu_score >= 10 || player_score >= 10) {
            speed_x = 0;
            speed_y = 0;
        } else {
            ResetBall();
        }
    }

    void ResetBall() {
        x = GetScreenWidth() / 2;
        y = GetScreenHeight() / 2;
        int speed_choices[2] = {-1, 1};
        int speed_increment = (player_score + cpu_score) / 2;
        speed_x = 7 + speed_increment;
        speed_y = 7 + speed_increment;
        speed_x *= speed_choices[GetRandomValue(0, 1)];
        speed_y *= speed_choices[GetRandomValue(0, 1)];
    }

    void ChangeColor() {
        color = Color{(unsigned char)GetRandomValue(50, 255), (unsigned char)GetRandomValue(50, 255), (unsigned char)GetRandomValue(50, 255), 255};
    }
};

class Paddle {
protected:
    void LimitMovement() {
        if (y <= 0) y = 0;
        if (y + height >= GetScreenHeight()) y = GetScreenHeight() - height;
    }

public:
    float x, y;
    float width, height;
    int speed;
    Texture2D racket;

    void Draw() {
        DrawRectangle(x,y,width,height, WHITE); // Adjust size
    }

    void Update() {
        int speed_increment = (player_score + cpu_score) / 4;
        int current_speed = speed + speed_increment;

        if (IsKeyDown(KEY_UP)) y -= current_speed;
        if (IsKeyDown(KEY_DOWN)) y += current_speed;

        LimitMovement();
    }
};

class CpuPaddle : public Paddle {
public:
    void Update(int ball_y) {
        int speed_increment = (player_score + cpu_score) / 4;
        int current_speed = speed + speed_increment;

        if (y + height / 2 > ball_y) y -= current_speed;
        if (y + height / 2 <= ball_y) y += current_speed;
    }
};

Ball ball;
Paddle player;
CpuPaddle cpu;

void RestartGame() {
    player_score = 0;
    cpu_score = 0;
    ball.ResetBall();
}

// 🎾 Main Function
int main() {
    const int screen_width = 1280;
    const int screen_height = 800;

    InitWindow(screen_width, screen_height, "Table Tennis Pong!");
    InitAudioDevice();

    Sound hitSound = LoadSound("aee.wav");
    Sound missSound = LoadSound("baigan.wav");

    ball.radius = 20;
    ball.x = screen_width / 2;
    ball.y = screen_height / 2;
    ball.speed_x = 7;
    ball.speed_y = 7;

    player.width = 25;
    player.height = 120;
    player.x = screen_width - player.width - 10;
    player.y = screen_height / 2 - player.height / 2;
    player.speed = 6;
    

    cpu.width = 25;
    cpu.height = 120;
    cpu.x = 10;
    cpu.y = screen_height / 2 - cpu.height / 2;
    cpu.speed = 6;
   

    Rectangle restartButton = {screen_width / 2 - 100, screen_height - 80, 200, 50};

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(Dark_Green);
        DrawRectangle(screen_width / 2, 0, screen_width / 2, screen_height, Green);
        DrawCircle(screen_width / 2, screen_height / 2, 150, Light_Green);
        DrawLine(screen_width / 2, 0, screen_width / 2, screen_height, WHITE);

        if (cpu_score >= 10) {
            DrawText("LEFT PLAYER WINS!", screen_width / 2 - 200, screen_height / 2 - 40, 50, WHITE);
        } else if (player_score >= 10) {
            DrawText("RIGHT PLAYER WINS!", screen_width / 2 - 200, screen_height / 2 - 40, 50, WHITE);
        } else {
            ball.Update(missSound, hitSound);
            player.Update();
            cpu.Update(ball.y);

            if (CheckCollisionCircleRec(Vector2{ball.x, ball.y}, ball.radius, Rectangle{player.x, player.y, player.width, player.height})) {
                ball.speed_x *= -1;
                PlaySound(hitSound);
                //ball.ChangeColor();
            }
            if (CheckCollisionCircleRec(Vector2{ball.x, ball.y}, ball.radius, Rectangle{cpu.x, cpu.y, cpu.width, cpu.height})) {
                ball.speed_x *= -1;
                PlaySound(hitSound);
                //ball.ChangeColor();
            }

            ball.Draw();
            cpu.Draw();
            player.Draw();

            DrawText(TextFormat("%i", cpu_score), screen_width / 4 - 20, 20, 80, WHITE);
            DrawText(TextFormat("%i", player_score), 3 * screen_width / 4 - 20, 20, 80, WHITE);
        }

        // Draw Restart Button
        DrawRectangleRec(restartButton, WHITE);
        DrawText("Restart", screen_width / 2 - 60, screen_height - 65, 30, BLACK);

        // Check for button click
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), restartButton)) {
            RestartGame();
        }

        EndDrawing();
    }

    UnloadSound(hitSound);
    UnloadSound(missSound);
    UnloadTexture(player.racket);
    UnloadTexture(cpu.racket);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
