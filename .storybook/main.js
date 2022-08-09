function getStories () {
  if (process.env['STORYBOOK_STORY_PATH']) {
    return process.env['STORYBOOK_STORY_PATH']
      .split(/\s+/)
      .map((path) => `../${path}`)
  }

  return [
    '../components/**/stories/*.tsx',
    '../components/**/*.stories.tsx'
  ]
}

module.exports = {
  stories: getStories(),
  addons: [
    '@storybook/addon-knobs',
    '@storybook/addon-essentials'
  ]
}
