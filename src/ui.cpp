#include <raylib.h>

#include <algorithm>
#include <generator>
#include <random>
#include <ranges>
#include <set>

import flit.game;
import flit.bots;

struct Coord
{
	int row;
	int col;
	bool operator==(Coord const &) const = default;
};

void
bot_select_list(
	std::unique_ptr<flit::bots::Bot> &selected_bot,
	std::size_t &selected_bot_idx,
	Font &font,
	float font_size,
	float spacing,
	float x,
	float y,
	float width,
	float height,
	Vector2 mouse_point)
{
	Vector2 dimensions = MeasureTextEx(font, "Free Play", font_size, spacing);
	DrawTextEx(
		font,
		"Free Play",
		{x + width / 2 - dimensions.x / 2, y + height / 2 - dimensions.y / 2},
		font_size,
		spacing,
		GRAY);

	Rectangle free_play_box = {x, y, width, height};
	if (CheckCollisionPointRec(mouse_point, free_play_box))
	{
		DrawRectangleRec(free_play_box, Fade(BLACK, 0.15));
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		{
			selected_bot = nullptr;
			selected_bot_idx = -1;
		}
	}
	if (selected_bot == nullptr)
	{
		DrawRectangleLinesEx(free_play_box, height * 0.05f, BLACK);
	}
	int idx = 0;
	for (auto &[name, make_bot] : flit::bots::bots)
	{
		Vector2 dimensions = MeasureTextEx(font, name.c_str(), font_size, spacing);
		DrawTextEx(
			font,
			name.c_str(),
			{x + width / 2 - dimensions.x / 2, y + (idx + 1.5f) * height - dimensions.y / 2},
			font_size,
			spacing,
			GRAY);

		Rectangle bot_select_box = {x, y + (idx + 1) * height, width, height};
		if (CheckCollisionPointRec(mouse_point, bot_select_box))
		{
			DrawRectangleRec(bot_select_box, Fade(BLACK, 0.15));

			if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
			{
				selected_bot = make_bot();
				selected_bot_idx = idx;
			}
		}
		if (selected_bot_idx == idx)
		{
			DrawRectangleLinesEx(bot_select_box, height * 0.05f, BLACK);
		}
		++idx;
	}
};

int
main()
{
	std::random_device rd{};
	std::mt19937 gen{rd()};

	InitWindow(2400, 1600, "Flit");
	SetExitKey(KEY_NULL);
	SetTargetFPS(60);

	Font font = GetFontDefault();

	flit::GameState game;
	game.turn(flit::Cell::Green);
	std::optional<Coord> selected{};
	std::vector<flit::Move> moves{};
	Color highlight_color = BLACK;
	std::unique_ptr<flit::bots::Bot> green_bot;
	std::size_t green_bot_idx = -1;
	std::unique_ptr<flit::bots::Bot> purple_bot;
	std::size_t purple_bot_idx = -1;

	auto const maybe_spawn_blue = [&]
	{
		std::uniform_int_distribution roll{1, 6};
		if (roll(gen) == 1)
		{
			auto possible_spawns = game.get_possible_spawns() | std::ranges::to<std::vector>();
			if (not possible_spawns.empty())
			{
				std::uniform_int_distribution<std::size_t> dist{0, possible_spawns.size() - 1};
				game.set(possible_spawns[dist(gen)], flit::Cell::Blue);
			}
		}
	};

	while (!WindowShouldClose())
	{
		auto legal_moves = game.get_legal_moves();
		std::optional<flit::Cell> winner;
		if (game.green_count() >= 48)
		{
			winner = flit::Cell::Green;
		}
		else if (game.purple_count() >= 48)
		{
			winner = flit::Cell::Purple;
		}
		else if (not winner.has_value() and legal_moves.begin() != legal_moves.end())
		{
			if (game.turn() == flit::Cell::Green and green_bot != nullptr)
			{
				flit::Move move = green_bot->choose_move(game);
				game.commit(move);
				maybe_spawn_blue();
			}

			if (game.turn() == flit::Cell::Purple and purple_bot != nullptr)
			{
				flit::Move move = purple_bot->choose_move(game);
				game.commit(move);
				maybe_spawn_blue();
			}
		}
		else
		{
			winner = flit::opponent(game.turn());
		}

		int const screen_height = GetScreenHeight();
		int const screen_width = GetScreenWidth();

		float const cell_size =
			static_cast<float>(std::min(screen_height, screen_width)) / std::max(flit::rows, flit::cols);
		float const font_size = cell_size / 3;
		float const spacing = 8;

		Vector2 mouse_point = GetMousePosition();

		BeginDrawing();
		ClearBackground(RAYWHITE);

		// Grid
		for (int row = 0; row < flit::rows + 1; ++row)
		{
			int y = row * cell_size;
			DrawLine(0, y, cell_size * flit::cols, y, GRAY);
		}

		for (int col = 0; col < flit::cols + 1; ++col)
		{
			int x = col * cell_size;
			DrawLine(x, 0, x, cell_size * flit::rows, GRAY);
		}

		for (std::uint_fast8_t row = 0; row < flit::rows; ++row)
		{
			for (std::uint_fast8_t col = 0; col < flit::cols; ++col)
			{
				float x = (col + 0.5f) * cell_size;
				float y = (row + 0.5f) * cell_size;

				Rectangle cell_box = {col * cell_size, row * cell_size, cell_size, cell_size};

				auto move_iter =
					std::ranges::find_if(moves, [&](flit::Move move) { return move.to == flit::from_rc(row, col); });

				bool is_hovered = CheckCollisionPointRec(mouse_point, cell_box);
				bool is_selected = selected.has_value() and selected.value() == Coord{row, col};

				Color color;
				switch (game.get(row, col))
				{
				case flit::Cell::Empty:
				{
					color = BLACK;
					int row_label = row + 1;
					char col_label = col + 'A';
					char const *text = TextFormat("%c%i", col_label, row_label);
					Vector2 dimensions = MeasureTextEx(font, text, font_size, spacing);
					DrawTextEx(font, text, {x - dimensions.x / 2, y - dimensions.y / 2}, font_size, spacing, GRAY);
					break;
				}
				case flit::Cell::Green:
				{
					color = GREEN;
					float scale = 0.5f;
					float offset = 0.125f;
					DrawTriangle(
						{x, y + cell_size * (-scale + offset)},
						{x - cell_size * scale * 0.866f, y + cell_size * (scale * 0.5f + offset)},
						{x + cell_size * scale * 0.866f, y + cell_size * (scale * 0.5f + offset)},
						GREEN);
					break;
				}
				case flit::Cell::Purple:
				{
					color = PURPLE;
					DrawPoly({x, y + 0.04f * cell_size}, 5, cell_size * 0.45f, -90, PURPLE);
					break;
				}
				case flit::Cell::Blue:
				{
					color = BLUE;
					float rect_size = cell_size * 0.75f;
					DrawRectangle(x - rect_size / 2, y - rect_size / 2, rect_size, rect_size, BLUE);
					break;
				}
				}

				if (is_selected)
				{
					DrawRectangleLinesEx(cell_box, cell_size * 0.05f, color);
					DrawRectangleRec(cell_box, Fade(color, 0.15f));
				}

				if (move_iter != moves.end())
				{
					DrawRectangleRec(cell_box, Fade(highlight_color, 0.15f));
					if (CheckCollisionPointRec(mouse_point, cell_box) and IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
					{
						game.commit(*move_iter);
						maybe_spawn_blue();
					}
				}

				if (is_hovered)
				{
					DrawRectangleRec(cell_box, Fade(color, 0.15f));

					if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
					{
						selected = {row, col};
						moves = game.get_legal_moves()
							| std::views::filter([&](flit::Move move) { return move.from == flit::from_rc(row, col); })
							| std::ranges::to<std::vector>();
						highlight_color = color;
					}
				}
			}
		}

		if (winner.has_value())
		{
			char const *message;
			Color color;
			switch (winner.value())
			{
			case flit::Cell::Green:
				color = GREEN;
				message = TextFormat("Green Wins %i-%i", game.green_count(), game.purple_count());
				break;
			case flit::Cell::Purple:
				color = PURPLE;
				message = TextFormat("Purple Wins %i-%i", game.green_count(), game.purple_count());
				break;
			}
			DrawRectangle(0, 0, cell_size * flit::cols, cell_size * flit::rows, Fade(color, 0.1));
			Vector2 dimensions = MeasureTextEx(font, message, font_size * 2, spacing);
			DrawTextEx(
				font,
				message,
				{cell_size * flit::cols / 2 - dimensions.x / 2, cell_size * flit::rows / 2 - dimensions.y / 2},
				font_size * 2,
				spacing,
				color);
		}

		// Menu
		float grid_width = cell_size * flit::cols;
		float extra_width = screen_width - grid_width;

		Vector2 dimensions = MeasureTextEx(font, "New Game", font_size, spacing);
		DrawTextEx(
			font,
			"New Game",
			{grid_width + extra_width / 2 - dimensions.x / 2, cell_size / 2 - dimensions.y / 2},
			font_size,
			spacing,
			GRAY);

		Rectangle new_game_box = {grid_width, 0, extra_width, cell_size};
		if (CheckCollisionPointRec(mouse_point, new_game_box))
		{
			DrawRectangleRec(new_game_box, Fade(BLACK, 0.15));
			if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
			{
				game = flit::GameState{};
				game.turn(flit::Cell::Green);
				std::uniform_int_distribution<int> row_dist{0, flit::rows - 1};
				std::uniform_int_distribution<int> col_dist{0, flit::cols - 1};

				auto try_set = [&](flit::Cell cell)
				{
					std::uint_fast8_t row;
					std::uint_fast8_t col;
					do
					{
						row = row_dist(gen);
						col = col_dist(gen);
					} while (game.get(row, col) != flit::Cell::Empty);
					game.set(row, col, cell);
				};
				try_set(flit::Cell::Green);
				try_set(flit::Cell::Green);
				try_set(flit::Cell::Purple);
				try_set(flit::Cell::Purple);
			}
		}

		bot_select_list(
			green_bot,
			green_bot_idx,
			font,
			font_size,
			spacing,
			grid_width,
			cell_size,
			extra_width / 2,
			cell_size,
			mouse_point);
		bot_select_list(
			purple_bot,
			purple_bot_idx,
			font,
			font_size,
			spacing,
			grid_width + extra_width / 2,
			cell_size,
			extra_width / 2,
			cell_size,
			mouse_point);

		EndDrawing();
	}

	CloseWindow();
}