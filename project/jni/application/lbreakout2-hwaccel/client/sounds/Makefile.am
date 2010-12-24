EXTRA_DIST = attach.wav click.wav exp.wav expand.wav extraball.wav \
freeze.wav gainlife.wav goldshower.wav joker.wav looselife.wav metal.wav \
menu_motion.wav reflect_paddle.wav reflect_brick.wav score.wav shot.wav shrink.wav speeddown.wav \
speedup.wav standard.wav wall.wav menu_click.wav \
dammit.wav damn.wav excellent.wav verygood.wav wontgiveup.wav \
chaos.wav darkness.wav ghost.wav timeadd.wav expl_ball.wav weak_ball.wav \
bonus_magnet.wav malus_magnet.wav disable.wav

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(inst_dir)/sounds
	@for file in *.wav; do\
	  $(INSTALL_DATA) $$file $(DESTDIR)$(inst_dir)/sounds/$$file;\
	done

win32-install-script:
	@for file in $(EXTRA_DIST); do \
	  echo "Source: \"`pwd -W`\\$$file\"; DestDir: \"{app}\\sounds\"; CopyMode: alwaysoverwrite" >> @win32_inst_script@; \
	done
