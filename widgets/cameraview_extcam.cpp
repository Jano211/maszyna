/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "stdafx.h"
#include "widgets/cameraview_extcam.h"
#include "stb/stb_image.h"
#include "Globals.h"
#include "Logs.h"
#include "extras/piped_proc.h"

ui::cameraview_panel::cameraview_panel()
    : ui_panel(STR_C("Camera preview"), false)
{
	size_min = { -2, -2 };
}

void cameraview_window_callback(ImGuiSizeCallbackData *data) {
	data->DesiredSize.y = data->DesiredSize.x * (float)Global.extcam_res.y / (float)Global.extcam_res.x;
}

ui::cameraview_panel::~cameraview_panel() {
	exit_thread = true;
	if (workthread.joinable())
		workthread.detach();
}

void ui::cameraview_panel::render()
{
	if (!is_open && state != RECORDING) {
		exit_thread = true;
	}

	if (exit_thread) {
		if (workthread.joinable())
			workthread.join();
		exit_thread = false;
		if (state != IDLE && !Global.extcam_cmd.empty())
			workthread = std::thread(&cameraview_panel::workthread_func, this);
	}

	if (!is_open)
		return;

	ImGui::SetNextWindowSizeConstraints(ImVec2(200, 200), ImVec2(2500, 2500), cameraview_window_callback);

	auto const panelname{(title.empty() ? m_name : title) + "###" + m_name};
	if (ImGui::Begin(panelname.c_str(), &is_open)) {
		render_contents();
	}

	ImGui::End();
}

bool ui::cameraview_panel::set_state(state_e e)
{
	if (state != e) {
		exit_thread = true;
		state = e;
		is_open = (state != IDLE);
		return true;
	}
	return false;
}

void ui::cameraview_panel::render_contents()
{
	if (!texture) {
		texture.emplace();
		texture->make_stub();
	}

	texture->bind(0);

	{
		std::lock_guard<std::mutex> lock(mutex);
		if (image_ptr) {

			glActiveTexture(GL_TEXTURE0);

			glTexImage2D(
			    GL_TEXTURE_2D, 0,
			    GL_SRGB8_ALPHA8,
			    Global.extcam_res.x, Global.extcam_res.y, 0,
			    GL_RGB, GL_UNSIGNED_BYTE, image_ptr);
			image_ptr = nullptr;

			if (Global.python_mipmaps) {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glGenerateMipmap(GL_TEXTURE_2D);
			}
			else {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			}
		}
	}

	ImVec2 surface_size = ImGui::GetContentRegionAvail();
	ImGui::Image(reinterpret_cast<void*>(texture->id), surface_size);
}

void ui::cameraview_panel::workthread_func()
{
	std::string cmdline = Global.extcam_cmd;
	if (state == RECORDING)
		cmdline += " " + Global.extcam_rec;

	if (!rec_name.empty()) {
		const std::string magic{"{RECORD}"};
		size_t pos = cmdline.find(magic);
		if (pos != -1)
			cmdline.replace(pos, magic.size(), rec_name);
	}

	piped_proc proc(cmdline);

	size_t frame_size = Global.extcam_res.x * Global.extcam_res.y * 3;
	uint8_t *read_buffer = new uint8_t[frame_size];
	uint8_t *active_buffer = new uint8_t[frame_size];

	size_t bufpos = 0;
	while (!exit_thread) {
		size_t read = 0;
		read = proc.read(read_buffer + bufpos, frame_size - bufpos);

		if (!read)
			break;
		bufpos += read;

		if (bufpos != frame_size)
			continue;

		bufpos = 0;

		std::lock_guard<std::mutex> lock(mutex);
		image_ptr = read_buffer;
		read_buffer = active_buffer;
		active_buffer = image_ptr;
	}

	std::lock_guard<std::mutex> lock(mutex);
	image_ptr = nullptr;
	delete[] read_buffer;
	delete[] active_buffer;
}
