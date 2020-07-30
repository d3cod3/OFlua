--
--
--	----------------------------------------------------------
--	OFlua | OF lua live coding
--
--	Copyright (c) 2020 Emanuele Mazza aka n3m3da
--
--	OFlua is distributed under the MIT License.
--	This gives everyone the freedoms to use Mosaic in any context:
--	commercial or non-commercial, public or private,
--	open or closed source.
--
--  See https://github.com/d3cod3/OFlua for source code
--	----------------------------------------------------------
--
--
--	empty.lua: the basic OFlua template

---------------------------------- MOSAIC SPECIFIC CODING INFO

-- Mosaic system variable for loading external resources (files)
-- Example:
-- img = of.Image()
-- img:load(SCRIPT_PATH .. "/data/test.jpg")

---------------------------------- MOSAIC SPECIFIC CODING INFO


----------------------------------- MY VARS

-- add here your variables

----------------------------------- MY VARS

----------------------------------------------------

-- main methods

----------------------------------------------------
function setup()

	-- add here your setup code

end

----------------------------------------------------
function update()

	-- add here your update code

end

----------------------------------------------------
function draw()

	-- draw background first
	of.setColor(0,0,0,255)
	of.drawRectangle(0,0,OUTPUT_WIDTH,OUTPUT_HEIGHT)

	-- add here your draw code


end

----------------------------------------------------
function exit()

end

----------------------------------------------------

-- input callbacks

----------------------------------------------------
function keyPressed(key)

end

----------------------------------------------------
function keyReleased(key)

end

----------------------------------------------------
function mouseMoved(x, y)

end

----------------------------------------------------
function mouseDragged(x, y, button)

end

----------------------------------------------------
function mousePressed(x, y, button)

end

----------------------------------------------------
function mouseReleased(x, y, button)

end

----------------------------------------------------
function mouseScrolled(x, y, scrollX, scrollY)

end
