# 전술 디펜스 스프라이트

2026-10-15, 내장 `image_gen`으로 생성하고 같은 도구로 프레임 여백을 보정했다. CLI/API 별도 호출이나 후처리 이미지 편집은 하지 않았다. 생성 PNG를 그대로 복사해 투명도를 보존한다.

## 저장과 사용

- `No_Save_Love/resource/defense/allies.png`: 방패병, 저격수, 폭죽포, 충전기.
- `No_Save_Love/resource/defense/enemies.png`: 일반병, 질주병, 중장갑, 작은 무리.
- `No_Save_Love/resource/defense/specials.png`: 거대 파수꾼, 치유병, 포격병, 방해병.
- 배포 폴더 `No_Save_Love_Distribution/resource/defense/`에도 동일 PNG를 포함한다.

각 이미지는 1254×1254 RGBA, 4행×4열이다. GDI+의 실수 원본 사각형으로 너비/높이의 1/4씩 읽는다. 첫 두 열은 대기/걸음, 세 번째는 공격/능력, 네 번째는 피격이다. 적 걸음은 이동 거리로 진행하고, 아군 배치 시 짧은 진입 이동을 적용한다. 공격·능력·피격 프레임은 실제 전투 사건에 연결하며 ESC와 강화 선택 중에는 전투 시각도 멈춘다.

## 생성 프롬프트

각 시트의 실제 프롬프트는 공통 앞부분 + 해당 Subject + 공통 뒷부분 순서다. `transparent_background: true`를 사용했다.

### 공통 앞부분

Use case: stylized-concept. Production game sprite animation atlas for a WinAPI lane defense game. Create ONE square transparent PNG spritesheet with EXACTLY 4 columns and 4 rows of equal square cells (16 sprites).

### allies Subject

Row 1: cyan helmeted shield knight with large cyan shield on RIGHT. Row 2: amber cap sharpshooter carrying long rifle pointed RIGHT. Row 3: pink wheeled fireworks cannon pointing RIGHT. Row 4: purple little battery robot with two legs, electrical coil and golden lightning symbol. All allied characters face RIGHT. Columns 1 and 2: two different gentle idle / step poses. Column 3: strong attack pose (knight thrust, rifle recoil and tiny flash, cannon recoil and tiny spark, battery generating energy). Column 4: recoil when hit. Four frames depict IDENTICAL character design per row.

### enemies Subject

Row 1: red tin soldier robot with a short blade. Row 2: orange light speedy running robot with swept-back fins. Row 3: dark steel heavily armored bulky robot with golden armor plates. Row 4: small lime green round swarm bug robot with four legs. All enemy characters face LEFT. Columns 1 and 2: clear alternating walking poses with different leg positions. Column 3: melee attack pose swinging toward LEFT. Column 4: recoil when hit. Four frames depict IDENTICAL character design per row.

### specials Subject

Row 1: burgundy and gold massive crowned mechanical guardian boss, stocky body and two fists. Row 2: mint green medic robot with medical backpack and clear white plus symbol, radiating a small mint healing pulse in attack frame. Row 3: orange and charcoal mobile mortar robot with large cannon aimed LEFT, firing in attack frame. Row 4: violet mischievous jammer drone with tiny floating antenna and lightning claw, electric pulse in attack frame. All enemies face LEFT. Columns 1 and 2: alternating walking or floating poses. Column 3: attack or ability pose. Column 4: recoil when hit. Four frames depict IDENTICAL character design per row.

### 공통 뒷부분

Style: polished hand-painted chibi arcade robots, crisp dark outlines, beautiful cel shading, tactile metal, highly readable silhouette, side view with slight three-quarter front detail, consistent light from top left. Layout critical: each character fully inside its own equal-sized cell, centered horizontally and vertically; same scale and baseline between the 4 frames of each row; generous transparent margins around EVERY sprite; no overlapping cells. Each sprite occupies about 72% of cell width and 76% of height. Uniform invisible grid, zero padding outside the 4x4 grid. No labels, no words, no cell borders, no backgrounds, no scenery, no floor, no cast shadows, no checkerboard drawing. Real transparent alpha. Four visibly different frames for each row's character.

### 최종 여백 보정

각 생성 원본을 `referenced_image_paths`로 전달하고 아래 프롬프트로 수정했다. 결과를 실제 게임 화면에서 확인했다.

Edit this game spritesheet. Preserve the EXACT 4 rows, 4 columns, designs, facing directions and animation poses. Fix sprite atlas spacing for game engine slicing: each sprite MUST fit entirely inside its own equal quarter-width by quarter-height cell, with at least 12% of cell dimension fully transparent margin on ALL four sides of each cell. Shrink every sprite uniformly to achieve this. No sword, muzzle flash, coil, antenna, glowing effect or limb may touch a cell boundary or extend into a neighboring cell. Keep identical center and baseline across all 4 frames of a row. Perfect 4x4 equal grid over the entire square canvas, no external padding. DO NOT add visible grid lines or text. Keep actual transparent background. Nothing other than the 16 smaller isolated sprites.

