#pragma once

#include "components/simple_scene.h"


namespace m1
{
    class Tema1 : public gfxc::SimpleScene
    {
    public:
        struct ViewportSpace
        {
            ViewportSpace() : x(0), y(0), width(1), height(1) {}
            ViewportSpace(int x, int y, int width, int height)
                : x(x), y(y), width(width), height(height) {}
            int x;
            int y;
            int width;
            int height;
        };

        struct LogicSpace
        {
            LogicSpace() : x(0), y(0), width(1), height(1) {}
            LogicSpace(float x, float y, float width, float height)
                : x(x), y(y), width(width), height(height) {}
            float x;
            float y;
            float width;
            float height;
        };

        struct Plane
        {
            Plane() : x(0), y(0), width(1), height(1) {}
            Plane(float x, float y, float width, float height)
                : x(x), y(y), width(width), height(height) {}
            float x;
            float y;
            float width;
            float height;
        };

        struct Obstacle
        {
            Obstacle() : x(0), y(0), scaleX(1), scaleY(1) {}
            Obstacle(float x, float y, float scaleX, float scaleY)
                : x(x), y(y), scaleX(scaleX), scaleY(scaleY) {}
            float x;
            float y;
            float scaleX;
            float scaleY;
        };

        struct Enemy
        {
            Enemy() : x(0), y(0), angle(0), speed(0) {}
            Enemy(float x, float y, float angle, float speed)
                : x(x), y(y), angle(angle), speed(speed) {}
            float x;
            float y;
            float angle;
            float speed;
        };

        struct Projectile
        {
            Projectile() : x(0), y(0), angle(0), distance(0) {}
            Projectile(float x, float y, float angle, float distance)
                : x(x), y(y), angle(angle), distance(distance) {}
            float x;
            float y;
            float angle;
            float distance;
        };

    public:
        Tema1();
        ~Tema1();

        void Init() override;

    private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void DrawScene(glm::mat3 visMatrix);

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;

        glm::mat3 VisualizationTransf2D(const LogicSpace& logicSpace, const ViewportSpace& viewSpace);
        glm::mat3 VisualizationTransf2DUnif(const LogicSpace& logicSpace, const ViewportSpace& viewSpace);
        void CreateEnemies();
        void CreateObstacles();
        void DrawPlayer(glm::mat3 visMatrix);
        void DrawHealthbar(glm::mat3 visMatrix);
        void DrawEnemies(glm::mat3 visMatrix);

        void CheckMapCollision();
        void CheckEnemyCollision();
        bool CheckObstacleCollision(float newX, float newY, float length);
        bool CheckMapBoundary(float newX, float newY, float length);
        bool PointInArea(float posX, float posY, Obstacle* obstacle);

        void SetViewportArea(const ViewportSpace& viewSpace, glm::vec3 colorColor = glm::vec3(0), bool clear = true);

    protected:
        float length;
        float projectileLength;
        ViewportSpace viewSpace;
        LogicSpace logicSpace;
        Plane mapSquare;
        glm::mat3 modelMatrix, visMatrix;

        float playerX, playerY;
        float scaleX, scaleY;
        int timeAux, score, life;
        double projectileTime;  // projectile fire rate
        float maximumDistance;
        float projectileSpeed;
        float rotation;
        std::vector<Enemy> enemiesVector;
        std::vector<Obstacle> obstaclesVector;
        std::vector<Projectile> projectilesVector;
    };
}
