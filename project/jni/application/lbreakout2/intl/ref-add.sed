/^# Packages using this file: / {
  s/# Packages using this file://
  ta
  :a
  s/ lbreakout2 / lbreakout2 /
  tb
  s/ $/ lbreakout2 /
  :b
  s/^/# Packages using this file:/
}
