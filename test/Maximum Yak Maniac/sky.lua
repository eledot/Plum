vergeclass 'Sky' do
    function Sky:__init()
        self.clouds = {{}, {}}
        self.cloud_movement_speed = { 0.0625, 0.50 }
        self.spawn_offset = { plum.video.screenWidth + 50, plum.video.screenWidth + 100 }
    end
    
    function Sky:renderLayer(layer)
        plum.video.opacity = layer == 2 and 127 or 192
        for i, cloud in ipairs(self.clouds[layer]) do
            cloud.image:blit(cloud.x - world.x, cloud.y)
        end
        --resource.font.plain:PrintCenter(plum.video.screenWidth / 2, layer * 10, tostring(#self.clouds[layer]))
        plum.video.opacity = 255
    end
    
    function Sky:update()
        -- Spawn new clouds
        for layer = 1, 2 do
            if self.spawn_offset[layer] < world.x + plum.video.screenWidth then
                self.spawn_offset[layer] = world.x + plum.video.screenWidth
            end
  
            -- Move clouds
            for i, cloud in ipairs(self.clouds[layer]) do
                cloud.x = cloud.x - self.cloud_movement_speed[cloud.layer]
                if cloud.x + cloud.image.width < world.x - 10 then
                    cloud.dispose = true
                end
            end
        
            if #self.clouds[layer] < 2 then
                local c = nil
                local t = 0
                if layer == 1 then
                    c = Cloud(self.spawn_offset[layer], math.random(0,  80), layer)
                elseif layer == 2 then
                    c = Cloud(self.spawn_offset[layer], math.random(100,  plum.video.screenHeight - 80), layer)
                end
                if c then
                    if layer == 1 then
                        self.spawn_offset[layer] = self.spawn_offset[layer] + math.random(300, 600)
                    elseif layer == 2 then
                        self.spawn_offset[layer] = self.spawn_offset[layer] + math.random(600, 900)
                    end
                    table.insert(self.clouds[layer], c)
                end
            end
            
            -- Remove any clouds newly flagged for disposal, while avoiding
            -- the undefined behaviour that occurs if you
            -- you delete while iterating
            local i  = 1
            local t = self.clouds[layer]
            while i <= #t do
                if t[i].dispose then
                    table.remove(t, i)
                else
                    i = i + 1
                end
            end
        end
    end
end

vergeclass 'Cloud' do
    function Cloud:__init(x, y, layer)
        self.x = x
        self.y = y
        self.image = randomItem(resource.image.cloud)
        self.dispose = false
        self.layer = layer
    end
    
    function Cloud:touches(cloud)
        return self.x + self.image.width > cloud.x
                and self.x < cloud.x + cloud.image.width
                and self.y + self.image.height > cloud.y
                and self.y < cloud.y + cloud.image.height
                and self.layer == cloud.layer
    end
end