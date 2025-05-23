/*
 * Copyright (c) 2022-2023 Shenzhen Kaihong Digital Industry Development Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { Scr } from './engine/XDefine.js';
import { GLFrame } from './engine/GLFrame.js';
import { X2DFast } from './engine/graphics/X2DFast.js';
import { RandInt } from './engine/XTools.js';
import { XTexture } from './engine/graphics/XTexture.js';
const { NapiLog } = require('./hcs/NapiLog');
const { MainEditor } = require('./MainEditor');

let canvas = document.getElementById('visual_area');
let widthOffset = 420;
canvas.width = window.innerWidth - widthOffset;
canvas.height = window.innerHeight;

function myDraw() {
  if (
    canvas.width !== window.innerWidth - widthOffset ||
    canvas.height !== window.innerHeight
  ) {
    canvas.width = window.innerWidth - widthOffset;
    canvas.height = window.innerHeight;

    Scr.setLogicScreenSize(canvas.width, canvas.height);
    GLFrame.gi().resize();
  }

  let pm2f = X2DFast.gi();
  pm2f.swapMode2D();
  pm2f.clearBuffer();

  MainEditor.gi().draw(pm2f);

  pm2f.freshBuffer();
}

function myTouch(msg, x, y) {
  MainEditor.gi().onTouch(msg, x, y);
}

function myKey(type, code) {
  NapiLog.logError(type + code);
  MainEditor.gi().onKey(code);
}

Scr.setLogicScreenSize(canvas.width, canvas.height);
GLFrame.gi().go(canvas, myDraw, myTouch, myKey);
