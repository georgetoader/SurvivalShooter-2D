#include "lab_m1/Tema1/Tema1.h"

#include <vector>
#include <iostream>

#include "lab_m1/Tema1/transform2D.h"
#include "lab_m1/Tema1/object2D.h"

using namespace std;
using namespace m1;

#define RADIANS(angle)      ((angle) * 0.0174532925194444f)

/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


Tema1::Tema1()
{
    length = 1;
    projectileLength = 0.5;
    scaleX = 0.5;
    scaleY = 0.5;

    rotation = 0;
    timeAux = 0;
    projectileTime = 0;
    projectileSpeed = 0.2;
    maximumDistance = 8;
    score = 0;
    life = 4;

    playerX = 0;
    playerY = 0;
    modelMatrix = glm::mat3(1);
    visMatrix = glm::mat3(1);
}


Tema1::~Tema1()
{
}


void Tema1::Init()
{
    glm::ivec2 resolution = window->GetResolution();
    auto camera = GetSceneCamera();
    camera->SetOrthographic(0, (float)resolution.x, 0, (float)resolution.y, 0.01f, 400);
    camera->SetPosition(glm::vec3(0, 0, 50));
    camera->SetRotation(glm::vec3(0, 0, 0));
    camera->Update();
    GetCameraInput()->SetActive(false);

    logicSpace.x = 0;
    logicSpace.y = 0;
    logicSpace.width = 12;
    logicSpace.height = 12;

    glm::vec3 corner = glm::vec3(0.001, 0.001, 0);
    playerX = logicSpace.x + logicSpace.width / 2;
    playerY = logicSpace.y + logicSpace.height / 2;

    mapSquare.x = -10;
    mapSquare.y = -15;
    mapSquare.width = 30;
    mapSquare.height = 40;

    Mesh* plane = object2D::CreatePlane("plane", corner, mapSquare.width, mapSquare.height, glm::vec3(0.4f, 0.4f, 0.4f), true);
    AddMeshToList(plane);
    Mesh* obstacle = object2D::CreateSquare("obstacle", corner, length, glm::vec3(0, 0.6f, 0), true);
    AddMeshToList(obstacle);
    Mesh* player = object2D::CreateSquare("player", corner, length, glm::vec3(1, 1, 0), true);
    AddMeshToList(player);
    Mesh* enemy = object2D::CreateSquare("enemy", corner, length, glm::vec3(1, 0, 0), true);
    AddMeshToList(enemy);
    Mesh* projectile = object2D::CreateSquare("projectile", corner, projectileLength, glm::vec3(0, 0, 0), true);
    AddMeshToList(projectile);
    Mesh* triangle = object2D::CreateTriangle("triangle", corner, length, glm::vec3(0, 0, 0));
    AddMeshToList(triangle);
    Mesh* frame = object2D::CreateWireframe("frame", corner, length, glm::vec3(0, 0, 0));
    AddMeshToList(frame);
    // folosesc wireframe pentru ambele pentru ca are un Oz mai mare deci are prioritate la drawScene
    Mesh* healthbar1 = object2D::CreateWireframe("healthbar1", corner, length, glm::vec3(1, 0, 0));
    AddMeshToList(healthbar1);
    Mesh* healthbar2 = object2D::CreateWireframe("healthbar2", corner, length, glm::vec3(1, 0, 0), true);
    AddMeshToList(healthbar2);

    // creez obstacolele aici si nu in Update() deoarece sunt fixe si nu ar avea sens
    CreateObstacles();
}

void Tema1::CreateObstacles()
{
    // creez vectorul de obstacole cu pozitiile fixe
    Obstacle* obstacle = new Obstacle(-3, 8, 2, 7);
    obstaclesVector.push_back(*obstacle);
    obstacle = new Obstacle(6, -3, 7, 3);
    obstaclesVector.push_back(*obstacle);
    obstacle = new Obstacle(-7, -4, 7, 2);
    obstaclesVector.push_back(*obstacle);
    obstacle = new Obstacle(8, 8, 7, 4);
    obstaclesVector.push_back(*obstacle);
    obstacle = new Obstacle(3, 20, 10, 2);
    obstaclesVector.push_back(*obstacle);
    obstacle = new Obstacle(12, -10, 8, 2);
    obstaclesVector.push_back(*obstacle);
}


// 2D visualization matrix
glm::mat3 Tema1::VisualizationTransf2D(const LogicSpace& logicSpace, const ViewportSpace& viewSpace)
{
    float sx, sy, tx, ty;
    sx = viewSpace.width / logicSpace.width;
    sy = viewSpace.height / logicSpace.height;
    tx = viewSpace.x - sx * logicSpace.x;
    ty = viewSpace.y - sy * logicSpace.y;

    return glm::transpose(glm::mat3(
        sx, 0.0f, tx,
        0.0f, sy, ty,
        0.0f, 0.0f, 1.0f));
}

// Uniform 2D visualization matrix (same scale factor on x and y axes)
glm::mat3 Tema1::VisualizationTransf2DUnif(const LogicSpace& logicSpace, const ViewportSpace& viewSpace)
{
    float sx, sy, tx, ty, smin;
    sx = viewSpace.width / logicSpace.width;
    sy = viewSpace.height / logicSpace.height;
    if (sx < sy)
        smin = sx;
    else
        smin = sy;
    tx = viewSpace.x - smin * logicSpace.x + (viewSpace.width - smin * logicSpace.width) / 2;
    ty = viewSpace.y - smin * logicSpace.y + (viewSpace.height - smin * logicSpace.height) / 2;

    return glm::transpose(glm::mat3(
        smin, 0.0f, tx,
        0.0f, smin, ty,
        0.0f, 0.0f, 1.0f));
}

void Tema1::SetViewportArea(const ViewportSpace& viewSpace, glm::vec3 colorColor, bool clear)
{
    glViewport(viewSpace.x, viewSpace.y, viewSpace.width, viewSpace.height);

    glEnable(GL_SCISSOR_TEST);
    glScissor(viewSpace.x, viewSpace.y, viewSpace.width, viewSpace.height);

    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(colorColor.r, colorColor.g, colorColor.b, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);

    GetSceneCamera()->SetOrthographic((float)viewSpace.x, (float)(viewSpace.x + viewSpace.width), (float)viewSpace.y, (float)(viewSpace.y + viewSpace.height), 0.1f, 400);
    GetSceneCamera()->Update();
}


void Tema1::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


void Tema1::Update(float deltaTimeSeconds)
{
    glm::ivec2 resolution = window->GetResolution();

    // Sets the screen area where to draw
    viewSpace = ViewportSpace(0, 0, resolution.x, resolution.y);
    SetViewportArea(viewSpace, glm::vec3(0), true);

    // Compute the 2D visualization matrix
    visMatrix = glm::mat3(1);
    visMatrix *= VisualizationTransf2DUnif(logicSpace, viewSpace);

    // creez un nou inamic in fiecare secunda incepand cu secunda 2
    // la fiecare 5 secunde vine un val de 3 inamici in loc de 1
    double time = Engine::GetElapsedTime();
    if (time > timeAux + 2) {
        CreateEnemies();
        if (timeAux % 5 == 0 && timeAux) {
            CreateEnemies();
            CreateEnemies();
        }
        timeAux += 1;
    }
    // verificare coliziuni
    CheckMapCollision();
    CheckEnemyCollision();
    // jucatorul a murit deci jocul se termina
    if (life == 0) {
        printf("------------------------------\n");
        printf("GAME OVER!\nBETTER LUCK NEXT TIME!\nFINAL SCORE: %d\n", score);
        printf("------------------------------\n");
        World::Exit();
    }

    DrawScene(visMatrix);
}

void Tema1::CreateEnemies()
{
    // creeaza un inamic pe pozitie random din jurul playerului, cu viteza tot random
    // nu creez inamic in exteriorul hartii
    // un numar random -1 sau 1 pentru a avea inamici din orice directie
    int randNr = rand() % 2 == 0 ? -1 : 1;
    float enemyX = playerX + ((rand() % 4) + 4) * randNr;
    randNr = rand() % 2 == 0 ? -1 : 1;
    float enemyY = playerY + ((rand() % 4) + 4) * randNr;
    float enemyAngle = atan2(playerY - enemyY, playerX - enemyX);
    float speed = 0.03 + ((rand() % 8) / 100.0);
    if (CheckMapBoundary(enemyX, enemyY, length)) {
        Enemy* enemy = new Enemy(enemyX, enemyY, enemyAngle, speed);
        enemiesVector.push_back(*enemy);
    }
}

void Tema1::CheckMapCollision()
{
    // verifica daca un inamic a iesit din limitele hartii in urma miscarii
    for (int i = 0; i < enemiesVector.size(); i++) {
        enemiesVector[i].x += enemiesVector[i].speed * cos(enemiesVector[i].angle);
        enemiesVector[i].y += enemiesVector[i].speed * sin(enemiesVector[i].angle);
        // updatez unghiul pentru a urmari player-ul
        enemiesVector[i].angle = atan2(playerY - enemiesVector[i].y, playerX - enemiesVector[i].x);
        if (!CheckMapBoundary(enemiesVector[i].x, enemiesVector[i].y, length)) {

            enemiesVector.erase(enemiesVector.begin() + i);
            i--;
        }
    }
    // verifica daca un proiectil a iesit din limitele hartii in urma miscarii
    // SAU daca a depasit distanta maxima pe care o poate traversa pana sa dispara
    // SAU daca s-a lovit de un obstacol
    for (int i = 0; i < projectilesVector.size(); i++) {
        // cos si sin pentru directia miscarii proiectilului, la mine patratul este initial orientat inspre dreapta
        float newX = projectileSpeed * cos(projectilesVector[i].angle);
        float newY = projectileSpeed * sin(projectilesVector[i].angle);
        projectilesVector[i].x += newX;
        projectilesVector[i].y += newY;
        projectilesVector[i].distance += sqrt(pow(newX, 2) + pow(newY, 2));
        if (!CheckMapBoundary(projectilesVector[i].x, projectilesVector[i].y, projectileLength) ||
            projectilesVector[i].distance > maximumDistance ||
            !CheckObstacleCollision(projectilesVector[i].x, projectilesVector[i].y, projectileLength)) {

            projectilesVector.erase(projectilesVector.begin() + i);
            i--;
        }
    }
}

// verifica daca exista coliziuni intre inamici - proiectile SAU inamici - player
void Tema1::CheckEnemyCollision()
{
    float distance;
    float first, second, collisionDistance;
    bool collision;
    for (int i = 0; i < enemiesVector.size(); i++) {
        // variabila pentru verificarea existentei unei coliziuni pentru inamicul curent
        collision = false;
        // verifica coliziunea unui inamic cu proiectilele folosind distanta euclidiana dintre cele 2 centre
        distance = length / 2 + projectileLength / 2;
        for (int j = 0; j < projectilesVector.size(); j++) {
            first = pow(enemiesVector[i].x - projectilesVector[j].x, 2);
            second = pow(enemiesVector[i].y - projectilesVector[j].y, 2);
            collisionDistance = sqrt(first + second);
            if (collisionDistance <= distance) {
                score++;
                // la fiecare 5 inamici eliminati afisez scorul
                if (score % 5 == 0) {
                    printf("Scorul jucatorului: %d\n", score);
                }
                enemiesVector.erase(enemiesVector.begin() + i);
                projectilesVector.erase(projectilesVector.begin() + j);
                i--;
                // inamicul curent a fost eliminat in urma coliziunii
                collision = true;
                break;
            }
        }
        // daca inamicul curent nu a fost eliminat de un proiectil atunci pot verifica coliziunea cu player-ul
        if (!collision) {
            distance = length / 2 + length / 2;
            // verificare distanta euclidiana
            first = pow(enemiesVector[i].x - playerX, 2);
            second = pow(enemiesVector[i].y - playerY, 2);
            collisionDistance = sqrt(first + second);
            if (collisionDistance <= distance) {
                life--;
                enemiesVector.erase(enemiesVector.begin() + i);
                i--;
            }
        }
    }
}

// verifica daca figura data (patrat) a iesit din limitele hartii
bool Tema1::CheckMapBoundary(float newX, float newY, float length)
{
    if (newY + length / 2 > mapSquare.y + mapSquare.height || 
        newY - length / 2 < mapSquare.y || 
        newX + length / 2 > mapSquare.x + mapSquare.width || 
        newX - length / 2 < mapSquare.x) {

        return false;
    }

    return true;
}

// verifica daca exista coliziune intre figura data (patrat) si oricare dintre obstacole
// returneaza "false" daca exista coliziune, altfel "true"
bool Tema1::CheckObstacleCollision(float newX, float newY, float length)
{
    for (int i = 0; i < obstaclesVector.size(); i++) {
        // verific ca niciunul din colturile figurii sa se afle in interiorul obstacolului
        // toate figurile au laturile paralele cu axele Ox si Oy deci merge verificarea
        if (!PointInArea(newX - length / 2, newY - length / 2, &obstaclesVector[i]) ||
            !PointInArea(newX + length / 2, newY - length / 2, &obstaclesVector[i]) ||
            !PointInArea(newX - length / 2, newY + length / 2, &obstaclesVector[i]) ||
            !PointInArea(newX + length / 2, newY + length / 2, &obstaclesVector[i])) {

            return false;
        }
    }
    // nu exista coliziuni intre obstacole si obiectul verificat
    return true;
}

void Tema1::FrameEnd()
{
}

void Tema1::DrawScene(glm::mat3 visMatrix)
{
    // desenez harta
    modelMatrix = visMatrix * transform2D::Translate(mapSquare.x, mapSquare.y);
    RenderMesh2D(meshes["plane"], shaders["VertexColor"], modelMatrix);

    // desenez obstacolele
    for (int i = 0; i < obstaclesVector.size(); i++) {
        modelMatrix = visMatrix * transform2D::Translate(obstaclesVector[i].x, obstaclesVector[i].y);
        // scalez obstacolele pentru a avea diferite forme dreptunghiulare
        modelMatrix = modelMatrix * transform2D::Scale(obstaclesVector[i].scaleX, obstaclesVector[i].scaleY);
        RenderMesh2D(meshes["obstacle"], shaders["VertexColor"], modelMatrix);
    }

    // desenez jucator, healthbar, inamici
    DrawPlayer(visMatrix);
    DrawHealthbar(visMatrix);
    DrawEnemies(visMatrix);

    // desenez proiectilele
    for (int i = 0; i < projectilesVector.size(); i++) {
        // coordonatele coltului stanga-jos unde se deseneaza
        float cornerX = projectilesVector[i].x - projectileLength / 2;
        float cornerY = projectilesVector[i].y - projectileLength / 2;

        modelMatrix = visMatrix * transform2D::Translate(cornerX, cornerY);
        modelMatrix = modelMatrix * transform2D::Translate(scaleX / 2, scaleY / 2);
        // proiectilul este rotit pentru a fi orientat spre directia din care a fost lansat
        modelMatrix = modelMatrix * transform2D::Rotate(projectilesVector[i].angle);
        modelMatrix = modelMatrix * transform2D::Translate(-scaleX / 2, -scaleY / 2);
        RenderMesh2D(meshes["projectile"], shaders["VertexColor"], modelMatrix);
    }
}

void Tema1::DrawPlayer(glm::mat3 visMatrix)
{
    // coordonatele coltului stanga-jos unde se deseneaza
    float cornerX = playerX - length / 2;
    float cornerY = playerY - length / 2;

    // desenare figura principala pentru player, un patrat galben cu frame negru
    modelMatrix = visMatrix * transform2D::Translate(cornerX, cornerY);
    modelMatrix = modelMatrix * transform2D::Translate(scaleX, scaleY);
    modelMatrix = modelMatrix * transform2D::Rotate(rotation);
    modelMatrix = modelMatrix * transform2D::Translate(-scaleX, -scaleY);
    RenderMesh2D(meshes["player"], shaders["VertexColor"], modelMatrix);
    RenderMesh2D(meshes["frame"], shaders["VertexColor"], modelMatrix);

    // desenez cele 2 tunuri, am vrut sa arata ca un fel de nava spatiala din StarWars (Millennium Falcon)
    // folosesc functii trigonometrice pentru a pastra pozitia fata de figura principala 
    modelMatrix = visMatrix * transform2D::Translate(cornerX + 0.6 * cos(rotation), cornerY + 0.6 * sin(rotation));
    modelMatrix = modelMatrix * transform2D::Translate(scaleX, scaleY);
    modelMatrix = modelMatrix * transform2D::Rotate(rotation);
    modelMatrix = modelMatrix * transform2D::Scale(0.5, 0.3);
    modelMatrix = modelMatrix * transform2D::Translate(0, -0.8);
    modelMatrix = modelMatrix * transform2D::Translate(-scaleX, -scaleY);
    RenderMesh2D(meshes["player"], shaders["VertexColor"], modelMatrix);
    // refolosesc modelMatrix si doar mut figura mai sus
    modelMatrix = modelMatrix * transform2D::Translate(0, 1.6);
    RenderMesh2D(meshes["player"], shaders["VertexColor"], modelMatrix);

    // triunghi negru, propulsorul din spate
    modelMatrix = visMatrix * transform2D::Translate(cornerX - 0.5 * cos(rotation), cornerY - 0.5 * sin(rotation));
    modelMatrix = modelMatrix * transform2D::Translate(scaleX, scaleY);
    modelMatrix = modelMatrix * transform2D::Rotate(rotation);
    // rotesc astfel incat baza tringhiului sa fie in exterior
    modelMatrix = modelMatrix * transform2D::Rotate(RADIANS(270));
    modelMatrix = modelMatrix * transform2D::Scale(0.9, 0.3);
    modelMatrix = modelMatrix * transform2D::Translate(-scaleX, -scaleY);
    RenderMesh2D(meshes["triangle"], shaders["VertexColor"], modelMatrix);
}

void Tema1::DrawHealthbar(glm::mat3 visMatrix)
{
    // plasez dreptunghiul gol in dreapta-sus
    modelMatrix = visMatrix * transform2D::Translate(logicSpace.x + 12, logicSpace.y + logicSpace.height - 2);
    modelMatrix = modelMatrix * transform2D::Translate(scaleX, scaleY);
    modelMatrix = modelMatrix * transform2D::Scale(5.9, 1);
    modelMatrix = modelMatrix * transform2D::Translate(-scaleX, -scaleY);
    RenderMesh2D(meshes["healthbar1"], shaders["VertexColor"], modelMatrix);

    // dreptunghiul 2 o sa fie plasat la finalul primului dreptunghi, si rotit 180 pentru a creste spre stanga
    // scalarea spre stanga este data de numarul de vieti ramase (4 vieti = maxim) 
    modelMatrix = visMatrix * transform2D::Translate(logicSpace.x + 14.5, logicSpace.y + logicSpace.height - 2);
    modelMatrix = modelMatrix * transform2D::Translate(scaleX, scaleY);
    modelMatrix = modelMatrix * transform2D::Rotate(RADIANS(180));
    modelMatrix = modelMatrix * transform2D::Translate(-scaleX, -scaleY);
    modelMatrix = modelMatrix * transform2D::Scale(life * 1.5, 1);
    RenderMesh2D(meshes["healthbar2"], shaders["VertexColor"], modelMatrix);
}

void Tema1::DrawEnemies(glm::mat3 visMatrix)
{
    for (int i = 0; i < enemiesVector.size(); i++) {
        float enemyX = enemiesVector[i].x - length / 2;
        float enemyY = enemiesVector[i].y - length / 2;
        float enemyRot = enemiesVector[i].angle;

        // figura principala a inamicului, un patrat rosu orientat inspre player
        modelMatrix = visMatrix * transform2D::Translate(enemyX, enemyY);
        modelMatrix = modelMatrix * transform2D::Translate(scaleX, scaleY);
        modelMatrix = modelMatrix * transform2D::Rotate(enemyRot);
        modelMatrix = modelMatrix * transform2D::Translate(-scaleX, -scaleY);
        RenderMesh2D(meshes["enemy"], shaders["VertexColor"], modelMatrix);

        // patrate mai mici "estetice" desenate in directia in care priveste inamicul
        // folosesc functii trigonometrice pentru a pastra pozitia fata de figura principala 
        modelMatrix = visMatrix * transform2D::Translate(enemyX + 0.6 * cos(enemyRot), enemyY + 0.6 * sin(enemyRot));
        modelMatrix = modelMatrix * transform2D::Translate(scaleX, scaleY);
        modelMatrix = modelMatrix * transform2D::Rotate(enemyRot);
        modelMatrix = modelMatrix * transform2D::Scale(0.5, 0.5);
        modelMatrix = modelMatrix * transform2D::Translate(-0.3, -0.8);
        modelMatrix = modelMatrix * transform2D::Translate(-scaleX, -scaleY);
        RenderMesh2D(meshes["enemy"], shaders["VertexColor"], modelMatrix);
        RenderMesh2D(meshes["frame"], shaders["VertexColor"], modelMatrix);
        // refolosesc modelMatrix si doar mut figura mai sus
        modelMatrix = modelMatrix * transform2D::Translate(0, 1.6);
        RenderMesh2D(meshes["enemy"], shaders["VertexColor"], modelMatrix);
        RenderMesh2D(meshes["frame"], shaders["VertexColor"], modelMatrix);
    }
}

// verifica daca punctul determinat de cele 2 coordonate se afla in interiorul obstacolului dat ca parametru
bool Tema1::PointInArea(float posX, float posY, Obstacle *obstacle)
{
    float obsX = obstacle->x;
    float obsY = obstacle->y;
    float obsWidth = length * obstacle->scaleX;
    float obsHeight = length * obstacle->scaleY;
    if ((posY <= obsY + obsHeight) && (posY >= obsY) &&
        (posX <= obsX + obsWidth) && (posX >= obsX)) {
        return false;
    }
    return true;
}

void Tema1::OnInputUpdate(float deltaTime, int mods)
{
    // miscarea WASD, verific sa nu iasa din limitele hartii si 
    // sa nu intre in coliziuni cu obstacole
    if (window->KeyHold(GLFW_KEY_W)) {
        if (CheckMapBoundary(playerX, playerY + 4 * deltaTime, length) &&
            CheckObstacleCollision(playerX, playerY + 4 * deltaTime, length)) {

            logicSpace.y += 4 * deltaTime;
        }
    }

    if (window->KeyHold(GLFW_KEY_S)) {
        if (CheckMapBoundary(playerX, playerY - 4 * deltaTime, length) && 
            CheckObstacleCollision(playerX, playerY - 4 * deltaTime, length)) {

            logicSpace.y -= 4 * deltaTime;
        }
    }

    if (window->KeyHold(GLFW_KEY_A)) {
        if (CheckMapBoundary(playerX - 4 * deltaTime, playerY, length) &&
            CheckObstacleCollision(playerX - 4 * deltaTime, playerY, length)) {

            logicSpace.x -= 4 * deltaTime;
        }
    }

    if (window->KeyHold(GLFW_KEY_D)) {
        if (CheckMapBoundary(playerX + 4 * deltaTime, playerY, length) &&
            CheckObstacleCollision(playerX + 4 * deltaTime, playerY, length)) {

            logicSpace.x += 4 * deltaTime;
        }
    }
    // actualizez coordonatele jucatorului
    playerX = logicSpace.x + logicSpace.width / 2;
    playerY = logicSpace.y + logicSpace.height / 2;
}


void Tema1::OnKeyPress(int key, int mods)
{
}


void Tema1::OnKeyRelease(int key, int mods)
{
    // Add key release event
}

void Tema1::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // rotatia pentru urmarirea cursorului de catre player, folosesc arctg y/x
    rotation = atan2(-mouseY + viewSpace.height / 2, mouseX - viewSpace.width / 2);
}


void Tema1::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // lansare proiectile, fire rate de 0.3s
    // proiectilul pleaca din centrul jucatorului, in directia in care acesta priveste
    double time = Engine::GetElapsedTime();
    if (button == 1 && time - projectileTime >= 0.3) {
        projectileTime = time;
        Projectile* projectile = new Projectile(playerX, playerY, rotation, 0);
        projectilesVector.push_back(*projectile);
    }
}


void Tema1::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Tema1::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}
