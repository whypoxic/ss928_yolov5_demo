# Project libs

SET(SOC_LIBS

	# MPI_LIBS
	libss_mpi.a
	# ISP_SUPPORT
	libss_ae.a
	libss_isp.a
	libot_isp.a
	libss_awb.a
	libss_dehaze.a
	libss_extend_stats.a
	libss_drc.a
	libss_ldci.a
	libss_crb.a
	libss_bnr.a
	libss_calcflicker.a
	libss_ir_auto.a
	libss_acs.a
	libss_acs.a
	libsns_os08a20.a
	libsns_os05a10_2l_slave.a
	libsns_imx347_slave.a
	libsns_imx485.a
	libsns_os04a10.a
	libsns_os08b10.a
	# ss_hnr

	# AUDIO_LIBA
	libss_voice_engine.a
	libss_upvqe.a
	libss_dnvqe.a
	libaac_comm.a
	libaac_enc.a
	libaac_dec.a
	libaac_sbr_enc.a
	libaac_sbr_dec.a

	# memset_s memcpy_s
	libsecurec.a

	# HDMI lib
	libss_hdmi.a

	# SVP
	libss_ive.a
	libss_md.a
	libss_mau.a
	libss_dpu_rect.a
	libss_dpu_match.a
	libss_dsp.a
	libascend_protobuf.a
	libsvp_acl.a
	libprotobuf-c.a

	libacl_cblas.so
	libacl_retr.so
	libacl_tdt_queue.so
	libadump.so
	libaicpu_kernels.so
	libaicpu_processer.so
	libaicpu_prof.so
	libaicpu_scheduler.so
	libalog.so
	libascendcl.so
	libascend_protobuf.so
	libcce_aicore.so
	libcpu_kernels_context.so
	libcpu_kernels.so
	libc_sec.so
	libdrv_aicpu.so
	libdrvdevdrv.so
	libdrv_dfx.so
	liberror_manager.so
	libge_common.so
	libge_executor.so
	libgraph.so
	libmmpa.so
	libmsprofiler.so
	libmsprof.so
	libopt_feature.so
	libregister.so
	libruntime.so
	libslog.so
	libtsdclient.so

	libss_mcf.so
	libss_mcf_vi.so
	libss_pqp.so
)

add_definitions(-DSENSOR0_TYPE=SONY_IMX485_MIPI_8M_30FPS_12BIT)
add_definitions(-DUSE_NCNN_SIMPLEOCV)