<template>
	<view class="content">
		<image class="logo" src="/static/logo.png"></image>
		<view class="text-area">
			<text class="title">{{token}}</text>
		</view>
	</view>
</template>

<script>
	const {
		createCommonToken
	} = require('@/key.js')
	export default {
		data() {
			return {
				title: 'Hello',
				token: '',
			}
		},
		onLoad() {
			const params = {
				author_key: 'H6Hs71dMOk2/2A4qTeccvWYXv7mZSxT9Fo9xSuEpDb1DlpINfA2RkHPt+HimIo0E',
				version: '2022-05-01',
				user_id: '428478',
			}
			this.token = createCommonToken(params);
			console.log(this.token)
		},
		methods: {

		}
	}
</script>

<style>
	.content {
		display: flex;
		flex-direction: column;
		align-items: center;
		justify-content: center;
	}

	.logo {
		height: 200rpx;
		width: 200rpx;
		margin-top: 200rpx;
		margin-left: auto;
		margin-right: auto;
		margin-bottom: 50rpx;
	}

	.text-area {
		display: flex;
		justify-content: center;
	}

	.title {
		font-size: 36rpx;
		color: #8f8f94;
	}
</style>