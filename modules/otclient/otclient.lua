OTClient = { }

-- TODO: load and save configurations
function OTClient.init()
  g_window.move({ x=220, y=220 })
  g_window.resize({ width=800, height=600 })
  g_window.setTitle('OTClient')
  g_window.setIcon('otcicon.png')

  addEvent(g_window.show)
  return true
end

function OTClient.terminate()
  g_window.hide()
end