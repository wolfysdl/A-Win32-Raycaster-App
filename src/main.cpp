#include "main_def.h"
#include "engine.h"
#include "draw.h"

using namespace std;

// this is where the messages received from the OS (Windows) are treated (e.g. clicked the "X" button: message sent by OS -> APP: close the window)
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch (msg) {

		// idk why but this should come first before any input
		case WM_CLOSE: {
			bRunning = FALSE;
			PostQuitMessage(0);
		} break;

		default:
			// any messages we did not treat? pass them to a default function
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}

	return 0;
}

int WINAPI WinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd) {

	Engine _Engine;
	
	// check if there's some problem initializing the engine
	if (_Engine.Init(hInstance, WndProc) != 0)
	{
		MessageBoxA(_Engine.get_hwnd(), "Failed to initialize the Engine!", "[ERROR]", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}

	game_bitmap wolf3d_bmp = { 0 };

	if (load_bmp(".\\assets\\wolf3d_tex.bmpx", &wolf3d_bmp) != 0)
	{
		MessageBoxA(NULL, "Failed to load a bmpx file!", "[ERROR]", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}

	game_bitmap barrel_bmp = { 0 };

	if (load_bmp(".\\assets\\barrel.bmpx", &barrel_bmp) != 0)
	{
		MessageBoxA(NULL, "Failed to load a bmpx file!", "[ERROR]", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}

	/* INIT GAME */

	float player_posX = 5.0f;
	float player_posY = 5.0f;

	const float speed = 10.0f;

	float dir_X = -1.0f;
	float dir_Y = 0.0f;

	float plane_X = 0.0f;
	float plane_Y = 0.66f;

	auto time_1 = chrono::system_clock::now();
	auto time_2 = chrono::system_clock::now();

	while (bRunning) {

		if (GetAsyncKeyState(VK_ESCAPE) < 0)
			bRunning = FALSE;

		_Engine.msg_loop();
		
		time_2 = chrono::system_clock::now();
		chrono::duration<float> elapsed = time_2 - time_1;
		time_1 = time_2;
		float elapsed_time = elapsed.count();

		float rotSpeed = 3.0f * elapsed_time;

		// clear screen
		FillRectangle(0, 0, WIN_WIDTH, WIN_HEIGHT / 2, 0x222222);
		FillRectangle(0, WIN_HEIGHT / 2, WIN_WIDTH, WIN_HEIGHT / 2, 0x555555);


		/* 2.5D RENDERING FROM HERE */

		// for every ray casted, do this until we have crossed all the width of the screen
		for(int x = 0; x < WIN_WIDTH; x++)
		{
			// camera x coordinates
			float camera_X = 2 * x / float(WIN_WIDTH) - 1;

			float ray_dirX = dir_X + plane_X * camera_X;
			float ray_dirY = dir_Y + plane_Y * camera_X;

			int map_X = int(player_posX);
			int map_Y = int(player_posY);

			float side_distX;
			float side_distY;

			float delta_distX = std::abs(1 / ray_dirX);
			float delta_distY = std::abs(1 / ray_dirY);

			float perp_wallDist;

			int step_X;
			int step_Y;

			bool hit = false;
			bool side = false;

			// finds which direction the ray points to
			if (ray_dirX < 0)
			{
				step_X = -1;
				side_distX = (player_posX - map_X) * delta_distX;
			}
			else
			{
				step_X = 1;
				side_distX = (map_X + 1.0f - player_posX) * delta_distX;
			}

			if (ray_dirY < 0)
			{
				step_Y = -1;
				side_distY = (player_posY - map_Y) * delta_distY;

			}
			else
			{
				step_Y = 1;
				side_distY = (map_Y + 1.0f - player_posY) * delta_distY;
			}

			// check and find which wall was hit
			while (hit == false)
			{
				if (side_distX < side_distY)
				{
					side_distX = side_distX + delta_distX;
					map_X = map_X + step_X;
					side = false;
				}
				else
				{
					side_distY = side_distY + delta_distY;
					map_Y = map_Y + step_Y;
					side = true;
				}

				if (map[map_X][map_Y] > 0)
				{
					hit = true;
				}
			}

			// calculating the distance between the plane and the wall
			if (side == false)
				perp_wallDist = (map_X - player_posX + (1 - step_X) / 2) / ray_dirX;
			else
				perp_wallDist = (map_Y - player_posY + (1 - step_Y) / 2) / ray_dirY;

			int line_height = int(WIN_HEIGHT / perp_wallDist);

			int draw_start = -line_height / 2 + WIN_HEIGHT / 2;

			if (draw_start < 0)
				draw_start = 0;

			int draw_end = line_height / 2 + WIN_HEIGHT / 2;

			if (draw_end >= WIN_HEIGHT)
				draw_end = WIN_HEIGHT - 1;

			// TODO: understand this stuff
			int tex_num = map[map_X][map_Y];

			float wall_X;	// where the wall was exactly hit

			if (side == false)
				wall_X = player_posY + perp_wallDist * ray_dirY;
			else
				wall_X = player_posX + perp_wallDist * ray_dirX;

			// calculates which part of the texture to draw (wall_X would go from 0 to 1)
			wall_X -= floorf(wall_X);
			int tex_X = int(wall_X * float(TILE_WIDTH));


			if (side == false && ray_dirX > 0)
				tex_X = (TILE_WIDTH - 1) - tex_X;

			if(side == true && ray_dirY < 0)
				tex_X = (TILE_WIDTH - 1) - tex_X;

			float step = 1.0f * TILE_HEIGHT / line_height;
			float tex_pos = (draw_start - WIN_HEIGHT / 2 + line_height / 2) * step;

			// loop for drawing a single scanline of texture
			for (int y = draw_start; y < draw_end; y++)
			{
				int tex_Y = int(tex_pos) & (TILE_HEIGHT - 1);
				tex_pos += step;

				uint32_t color = 0;

				switch (tex_num)
				{
					case 1:
					{
						LoadTextureIndex(&color, wolf3d_bmp, 1, 1, tex_X, tex_Y);
					}break;

					case 2:
					{
						LoadTextureIndex(&color, wolf3d_bmp, 3, 2, tex_X, tex_Y);
					}break;

					case 3:
					{
						LoadTextureIndex(&color, wolf3d_bmp, 4, 8, tex_X, tex_Y);
					}break;

					case 4:
					{
						LoadTextureIndex(&color, wolf3d_bmp, 2, 5, tex_X, tex_Y);
					}break;

					case 5:
					{
						LoadTextureIndex(&color, wolf3d_bmp, 3, 3, tex_X, tex_Y);
					}break;

					case 6:
					{
						LoadTextureIndex(&color, wolf3d_bmp, 5, 7, tex_X, tex_Y);
					}break;

					case 7:
					{
						LoadTextureIndex(&color, wolf3d_bmp, 3, 10, tex_X, tex_Y);
					}break;

					case 8:
					{
						LoadTextureIndex(&color, wolf3d_bmp, 5, 8, tex_X, tex_Y);
					}break;

					default:
						LoadTextureIndex(&color, wolf3d_bmp, 6, 10, tex_X, tex_Y);
						break;
				}
				
				if (side == true)
					color = (color >> 1) & 8355711;
				Draw(x, y, color);
			}
			z_buffer[x] = perp_wallDist;
		}

		for (int i = 0; i < MAX_SPRITES; i++)
		{
			sprite_order[i] = i;
			sprite_distance[i] = (player_posX - sprites[i].x) * (player_posX - sprites[i].x) + (player_posY - sprites[i].y) * (player_posY - sprites[i].y);
		}

		sort_sprites(sprite_order, sprite_distance, MAX_SPRITES);

		for (int i = 0; i < MAX_SPRITES; i++)
		{
			//translate sprite position to relative to camera
			double spriteX = sprites[sprite_order[i]].x - player_posX;
			double spriteY = sprites[sprite_order[i]].y - player_posY;

			//transform sprite with the inverse camera matrix
			// [ planeX   dirX ] -1                                       [ dirY      -dirX ]
			// [               ]       =  1/(planeX*dirY-dirX*planeY) *   [                 ]
			// [ planeY   dirY ]                                          [ -planeY  planeX ]

			double invDet = 1.0 / (plane_X * dir_Y - dir_X * plane_Y); //required for correct matrix multiplication

			double transformX = invDet * (dir_Y * spriteX - dir_X * spriteY);
			double transformY = invDet * (-plane_Y * spriteX + plane_X * spriteY); //this is actually the depth inside the screen, that what Z is in 3D

			int spriteScreenX = int((WIN_WIDTH / 2) * (1 + transformX / transformY));

			//calculate height of the sprite on screen
			int spriteHeight = abs(int(WIN_HEIGHT / (transformY))); //using 'transformY' instead of the real distance prevents fisheye
			//calculate lowest and highest pixel to fill in current stripe
			int drawStartY = -spriteHeight / 2 + WIN_HEIGHT / 2;
			if (drawStartY < 0) drawStartY = 0;
			int drawEndY = spriteHeight / 2 + WIN_HEIGHT / 2;
			if (drawEndY >= WIN_HEIGHT) drawEndY = WIN_HEIGHT - 1;

			//calculate width of the sprite
			int spriteWidth = abs(int(WIN_HEIGHT / (transformY)));
			int drawStartX = -spriteWidth / 2 + spriteScreenX;
			if (drawStartX < 0) drawStartX = 0;
			int drawEndX = spriteWidth / 2 + spriteScreenX;
			if (drawEndX >= WIN_WIDTH) drawEndX = WIN_WIDTH - 1;

			/*if (transformY > 0 && drawStartX > 0 && drawStartX < WIN_WIDTH && transformY < z_buffer[drawStartX])
			Blit32BMP(&barrel_bmp, drawStartX, drawStartY);*/

			//loop through every vertical stripe of the sprite on screen
			for (int stripe = drawStartX; stripe < drawEndX; stripe++)
			{
				int texX = int(256 * (stripe - (-spriteWidth / 2 + spriteScreenX)) * TILE_WIDTH / spriteWidth) / 256;
				//the conditions in the if are:
				//1) it's in front of camera plane so you don't see things behind you
				//2) it's on the screen (left)
				//3) it's on the screen (right)
				//4) ZBuffer, with perpendicular distance
				if (transformY > 0 && stripe > 0 && stripe < WIN_WIDTH && transformY < z_buffer[stripe])
					for (int y = drawStartY; y < drawEndY; y++) //for every pixel of the current stripe
					{
						int d = (y) * 256 - TILE_HEIGHT * 128 + spriteHeight * 128; //256 and 128 factors to avoid floats
						int texY = ((d * TILE_HEIGHT) / spriteHeight) / 256;

						uint32_t color = 0;

						LoadTextureIndex(&color, wolf3d_bmp, 5, 5, texX, texY);

						Draw(stripe, y, color);
					}
			}
		}

		/* HANDLING INPUT */
		if (GetAsyncKeyState(VK_D) < 0)
		{
			// rotate the view direction and plane
			float oldDirX = dir_X;
			dir_X = dir_X * cosf(-rotSpeed) - dir_Y * sinf(-rotSpeed);
			dir_Y = oldDirX * sinf(-rotSpeed) + dir_Y * cosf(-rotSpeed);
			float oldPlaneX = plane_X;
			plane_X = plane_X * cosf(-rotSpeed) - plane_Y * sinf(-rotSpeed);
			plane_Y = oldPlaneX * sinf(-rotSpeed) + plane_Y * cosf(-rotSpeed);
		}

		if (GetAsyncKeyState(VK_A) < 0)
		{
			// god I love math
			float oldDirX = dir_X;
			dir_X = dir_X * cosf(rotSpeed) - dir_Y * sinf(rotSpeed);
			dir_Y = oldDirX * sinf(rotSpeed) + dir_Y * cosf(-rotSpeed);
			float oldPlaneX = plane_X;
			plane_X = plane_X * cosf(rotSpeed) - plane_Y * sinf(rotSpeed);
			plane_Y = oldPlaneX * sinf(rotSpeed) + plane_Y * cosf(rotSpeed);
		}

		if (GetAsyncKeyState(VK_W) < 0)
		{
			// check ahead of player if there's a wall, if not move
			if(map[int(player_posX + dir_X)][int(player_posY)] == false)
				player_posX += dir_X * speed * elapsed_time;

			if(map[int(player_posX)][int(player_posY + dir_Y)] == false)
				player_posY += dir_Y * speed * elapsed_time;
		}

		if (GetAsyncKeyState(VK_S) < 0)
		{
			// check behind the player if there's a wall, if not move
			if (map[int(player_posX - dir_X)][int(player_posY)] == false)
				player_posX -= dir_X * speed * elapsed_time;

			if (map[int(player_posX)][int(player_posY - dir_Y)] == false)
				player_posY -= dir_Y * speed * elapsed_time;
		}

		// slap that backybuffer to the screen
		_Engine.blit_to_DIB();
	}

	return 0;
}