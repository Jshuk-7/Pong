#include "raylib.h"
#include <iostream>
#include <stdlib.h>

namespace Raylib::PongGame
{

    namespace Objects
    {
        enum Side
        {
            Left,
            Right,
        };

        enum Winner
        {
            LeftPaddle,
            RightPaddle,
            Undefined,
        };

        struct Window
        {
            const char *title;
            int width, height;
        };

        struct Paddle
        {
            float vel, height, width;
            Vector2 pos;
            Side side;

            Rectangle ToRect()
            {
                return Rectangle{this->pos.x - this->width / 2, this->pos.y - this->height / 2, this->width, this->height};
            }

            void GetInput()
            {
                if (this->side == Side::Left)
                {
                    if (IsKeyDown(KEY_W))
                    {
                        this->pos.y -= this->vel * GetFrameTime();
                        if (this->pos.y < 0)
                        {
                            this->pos.y = 0;
                        }
                    }
                    else if (IsKeyDown(KEY_S))
                    {
                        this->pos.y += this->vel * GetFrameTime();
                        if (this->pos.y > GetScreenHeight())
                        {
                            this->pos.y = GetScreenHeight();
                        }
                    }
                }
                else if (this->side == Side::Right)
                {
                    if (IsKeyDown(KEY_UP))
                    {
                        this->pos.y -= this->vel * GetFrameTime();
                        if (this->pos.y < 0)
                        {
                            this->pos.y = 0;
                        }
                    }
                    else if (IsKeyDown(KEY_DOWN))
                    {
                        this->pos.y += this->vel * GetFrameTime();
                        if (this->pos.y > GetScreenHeight())
                        {
                            this->pos.y = GetScreenHeight();
                        }
                    }
                }
            }

            void Draw()
            {
                DrawRectangleRec(ToRect(), WHITE);
            }
        };

        struct Ball
        {
            int radius;
            Vector2 pos, vel;

            //* Flips direction of the ball if it hits a wall
            //@param *posY Pointer to y position of the ball
            //@param *speedY Pointer to ball velocity
            static void SetBallBoundary(float *posY, float *speedY)
            {
                int height = GetScreenHeight();
                if (*posY < 0)
                {
                    *posY = 0;
                    *speedY *= -1;
                }
                if (*posY > height)
                {
                    *posY = height;
                    *speedY *= -1;
                }
            }

            static void CheckForCollision(Paddle leftPaddle, Paddle rightPaddle, Ball ball, float *velX)
            {
                Vector2 center{ball.pos.x, ball.pos.y};

                if (CheckCollisionCircleRec(center, ball.radius, leftPaddle.ToRect()))
                {
                    if (*velX < 0)
                    {
                        *velX *= -1;
                    }
                }
                if (CheckCollisionCircleRec(center, ball.radius, rightPaddle.ToRect()))
                {
                    if (*velX > 0)
                    {
                        *velX *= -1;
                    }
                }
            }

            static Winner CheckForWin(Vector2 pos, bool *gameOver)
            {
                if (pos.x < 0)
                {
                    *gameOver = true;
                    return Winner::LeftPaddle;
                }
                else if (pos.x > GetScreenWidth())
                {
                    *gameOver = true;
                    return Winner::RightPaddle;
                }
                return Winner::Undefined;
            }

            void Draw()
            {
                DrawCircle(this->pos.x, this->pos.y, this->radius, WHITE);
            }
        };
    } // namespace Objects

    void PrintToViewport(const char *TEXT)
    {
        DrawText(TEXT, (GetScreenWidth() / 2) / 1.45, GetScreenHeight() / 2 - 20, 25, BLACK);
    }

    void PrintToViewportAtPos(const char *TEXT, Vector2 pos)
    {
        DrawText(TEXT, pos.x, pos.y, 20, BLACK);
    }

    void ResetBall(float startVelX, float startVelY, float *ballVelX, float *ballVelY, float *ballX, float *ballY)
    {
        *ballVelX = startVelX;
        *ballVelY = startVelY;
        *ballX = GetScreenWidth() / 2;
        *ballY = GetScreenHeight() / 2;
    }

    void ResetPaddle(float *paddleY)
    {
        *paddleY = GetScreenHeight() / 2;
    }

    void RestartGame(bool *gameOver, Objects::Winner *winner)
    {
        if (IsKeyPressed(KEY_SPACE))
        {
            *gameOver = false;
            *winner = Objects::Winner::Undefined;
        }
    }

    void DisplayWinPanel(const char *TEXT)
    {
        ClearBackground(GRAY);
        PrintToViewport(TEXT);
        PrintToViewportAtPos("Press Space to restart the game", Vector2{(float)GetScreenWidth() / 2 / (float)2.10, (float)GetScreenHeight() / 2 + 50});
    }

    int Run()
    {
        using namespace Objects;

#pragma region Initialization

        //* Viewport settings
        Window window;
        window.title = "Pong Game";
        window.width = 640;
        window.height = 360;

        //* Ball transform
        int startVelX{150}, startVelY{300};
        Ball ball;
        ball.pos.x = window.width / 2;
        ball.pos.y = window.height / 2;
        ball.vel.x = startVelX;
        ball.vel.y = startVelY;
        ball.radius = 5;

        //* Win condition
        Winner winner;
        bool gameOver{false};

        //* Left Paddle transform
        Paddle leftPaddle;
        leftPaddle.width = 10;
        leftPaddle.height = 75;
        leftPaddle.pos.x = 15;
        leftPaddle.pos.y = window.height / 2;
        leftPaddle.vel = 500;
        leftPaddle.side = Side::Left;

        //* Right Paddle transform
        Paddle rightPaddle;
        rightPaddle.width = 10;
        rightPaddle.height = 75;
        rightPaddle.pos.x = window.width - 15;
        rightPaddle.pos.y = window.height / 2;
        rightPaddle.vel = 500;
        rightPaddle.side = Side::Right;

        //* FPS Text rect transform
        const int fpsPos{10};

        //* Instance window
        InitWindow(window.width, window.height, window.title);
        InitAudioDevice();
        SetWindowState(FLAG_VSYNC_HINT);

        //* Ambiance music
        Music music = LoadMusicStream("./audio/lady_hear_me_tonight.mp3");
        PlayMusicStream(music);

#pragma endregion

        while (!WindowShouldClose())
        {
            UpdateMusicStream(music);
            BeginDrawing();
            {
                if (!gameOver)
                {
                    ClearBackground(BLACK);

                    const float DELTA{GetFrameTime()}; // Time between each frame
                    ball.pos.x += ball.vel.x * DELTA;
                    ball.pos.y += ball.vel.y * DELTA;

                    ball.Draw();

                    Ball::SetBallBoundary(&ball.pos.y, &ball.vel.y);

                    Ball::CheckForCollision(leftPaddle, rightPaddle, ball, &ball.vel.x);

                    winner = Ball::CheckForWin(ball.pos, &gameOver);

                    leftPaddle.Draw();
                    leftPaddle.GetInput();

                    rightPaddle.Draw();
                    rightPaddle.GetInput();

                    DrawFPS(fpsPos, fpsPos);
                }
                else
                {
                    if (winner == Winner::LeftPaddle)
                    {
                        DisplayWinPanel("Right Player Won!");
                        ResetBall(startVelX, startVelY, &ball.vel.x, &ball.vel.y, &ball.pos.x, &ball.pos.y);
                        ResetPaddle(&leftPaddle.pos.y);
                        ResetPaddle(&rightPaddle.pos.y);
                        RestartGame(&gameOver, &winner);
                    }
                    else if (winner == Winner::RightPaddle)
                    {
                        DisplayWinPanel("Left Player Won!");
                        ResetBall(startVelX, startVelY, &ball.vel.x, &ball.vel.y, &ball.pos.x, &ball.pos.y);
                        ResetPaddle(&leftPaddle.pos.y);
                        ResetPaddle(&rightPaddle.pos.y);
                        RestartGame(&gameOver, &winner);
                        ball.vel.x *= -1;
                    }
                }
            }
            EndDrawing();
        }

        UnloadMusicStream(music);

        CloseAudioDevice();

        CloseWindow();

        return 0;
    }
} // namespace PongGame

int main()
{
    return Raylib::PongGame::Run();
}